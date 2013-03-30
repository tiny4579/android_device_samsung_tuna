/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOG_TAG "Tuna PowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

#define SCALINGMAXFREQ_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define SCREENOFFMAXFREQ_PATH "/sys/devices/system/cpu/cpu0/cpufreq/screen_off_max_freq"
#define SCALING_GOVERNOR_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
#define BOOSTPULSE_ONDEMAND "/sys/devices/system/cpu/cpufreq/ondemand/boostpulse"
#define BOOSTPULSE_KTOONSERVATIVE "/sys/devices/system/cpu/cpufreq/ktoonservative/boostpulse"
#define BOOSTPULSE_INTERACTIVE "/sys/devices/system/cpu/cpufreq/interactive/boostpulse"
#define SAMPLING_RATE_SCREEN_ON "50000"
#define SAMPLING_RATE_SCREEN_OFF "500000"
#define TIMER_RATE_SCREEN_ON "30000"
#define TIMER_RATE_SCREEN_OFF "500000"

#define MAX_BUF_SZ  10

/* initialize to something safe */
static char screen_off_max_freq[MAX_BUF_SZ] = "700000";
static char scaling_max_freq[MAX_BUF_SZ] = "1200000";

struct tuna_power_module {
    struct power_module base;
    pthread_mutex_t lock;
    int boostpulse_fd;
    int boostpulse_warned;
};

static char governor[20];

static void sysfs_write(char *path, char *s)
{
    char buf[80];
    int len;
    int fd = open(path, O_WRONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);
    }

    close(fd);
}

int sysfs_read(const char *path, char *buf, size_t size)
{
  int fd, len;

  fd = open(path, O_RDONLY);
  if (fd < 0)
    return -1;

  do {
    len = read(fd, buf, size);
  } while (len < 0 && errno == EINTR);

  close(fd);

  return len;
}

static int get_scaling_governor() {
    if (sysfs_read(SCALING_GOVERNOR_PATH, governor,
                sizeof(governor)) == -1) {
        return -1;
    } else {
        // Strip newline at the end.
        int len = strlen(governor);

        len--;

        while (len >= 0 && (governor[len] == '\n' || governor[len] == '\r'))
            governor[len--] = '\0';
    }

    return 0;
}

static void tuna_power_set_interactive(struct power_module *module, int on)
{
    if (strncmp(governor, "ondemand", 8) == 0)
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate",
                on ? SAMPLING_RATE_SCREEN_ON : SAMPLING_RATE_SCREEN_OFF);
    else if (strncmp(governor, "interactive", 11) == 0) 
    {
        int len;

        char buf[MAX_BUF_SZ];

        /*
        * Lower maximum frequency when screen is off.  CPU 0 and 1 share a
        * cpufreq policy.
        */
        if (!on) {
            /* read the current scaling max freq and save it before updating */
            len = sysfs_read(SCALINGMAXFREQ_PATH, buf, sizeof(buf));

            /* make sure it's not the screen off freq, if the "on"
            * call is skipped (can happen if you press the power
            * button repeatedly) we might have read it. We should
            * skip it if that's the case
            */
            if (len != -1 && strncmp(buf, screen_off_max_freq,
                    strlen(screen_off_max_freq)) != 0)
                memcpy(scaling_max_freq, buf, sizeof(buf));
            sysfs_write(SCALINGMAXFREQ_PATH, screen_off_max_freq);
        } else
            sysfs_write(SCALINGMAXFREQ_PATH, scaling_max_freq);
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/timer_rate",
                on ? TIMER_RATE_SCREEN_ON : TIMER_RATE_SCREEN_OFF);
    }
}

static void configure_governor()
{
    tuna_power_set_interactive(NULL, 1);

    if (strncmp(governor, "ondemand", 8) == 0) {
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/up_threshold", "90");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/io_is_busy", "1");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor", "4");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/down_differential", "10");

    } else if (strncmp(governor, "interactive", 11) == 0) {
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/min_sample_time", "60000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/go_hispeed_load", "75");
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/io_is_busy", "1");
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/hispeed_freq", "700000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/above_hispeed_delay", "30000");
    }
}

static void tuna_power_init(struct power_module *module)
{
    get_scaling_governor();
    configure_governor();
}

static int boostpulse_open(struct tuna_power_module *tuna)
{
    char buf[80];

    pthread_mutex_lock(&tuna->lock);

    if (tuna->boostpulse_fd < 0) {
        if (get_scaling_governor() < 0) {
            ALOGE("Can't read scaling governor.");
            tuna->boostpulse_warned = 1;
        } else {
            if (strncmp(governor, "ondemand", 8) == 0)
                tuna->boostpulse_fd = open(BOOSTPULSE_ONDEMAND, O_WRONLY);
            else if (strncmp(governor, "ktoonservative", 14) == 0)
                tuna->boostpulse_fd = open(BOOSTPULSE_KTOONSERVATIVE, O_WRONLY);
            else if (strncmp(governor, "interactive", 11) == 0)
                tuna->boostpulse_fd = open(BOOSTPULSE_INTERACTIVE, O_WRONLY);

            if (tuna->boostpulse_fd < 0 && !tuna->boostpulse_warned) {
                strerror_r(errno, buf, sizeof(buf));
                ALOGV("Error opening boostpulse: %s\n", buf);
                tuna->boostpulse_warned = 1;
            } else if (tuna->boostpulse_fd > 0) {
                configure_governor();
                ALOGD("Opened %s boostpulse interface", governor);
            }
        }
    }

    pthread_mutex_unlock(&tuna->lock);
    return tuna->boostpulse_fd;
}

static void tuna_power_hint(struct power_module *module, power_hint_t hint,
                            void *data)
{
    struct tuna_power_module *tuna = (struct tuna_power_module *) module;
    char buf[80];
    int len;
    int duration = 1;

    switch (hint) {
    case POWER_HINT_INTERACTION:
    case POWER_HINT_CPU_BOOST:
        if (boostpulse_open(tuna) >= 0) {
            if (data != NULL)
                duration = (int) data;

            snprintf(buf, sizeof(buf), "%d", duration);
            len = write(tuna->boostpulse_fd, buf, strlen(buf));

            if (len < 0) {
                strerror_r(errno, buf, sizeof(buf));
                ALOGE("Error writing to boostpulse: %s\n", buf);

                pthread_mutex_lock(&tuna->lock);
                close(tuna->boostpulse_fd);
                tuna->boostpulse_fd = -1;
                tuna->boostpulse_warned = 0;
                pthread_mutex_unlock(&tuna->lock);
            }
        }
        break;

    case POWER_HINT_VSYNC:
        break;

    default:
        break;
    }
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

struct tuna_power_module HAL_MODULE_INFO_SYM = {
    base: {
        common: {
            tag: HARDWARE_MODULE_TAG,
            module_api_version: POWER_MODULE_API_VERSION_0_2,
            hal_api_version: HARDWARE_HAL_API_VERSION,
            id: POWER_HARDWARE_MODULE_ID,
            name: "Tuna Power HAL",
            author: "The Android Open Source Project",
            methods: &power_module_methods,
        },

       init: tuna_power_init,
       setInteractive: tuna_power_set_interactive,
       powerHint: tuna_power_hint,
    },

    lock: PTHREAD_MUTEX_INITIALIZER,
    boostpulse_fd: -1,
    boostpulse_warned: 0,
};

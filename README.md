CM10.1 for the Galaxy Nexus

## Info
[**XDA Discussion thread**](http://forum.xda-developers.com/showthread.php?t=2235365)

### Initialize
[Setup Linux/OS X](http://source.android.com/source/initializing.html) - Please note: it must be sun-java-6, not openjdk

### Prepare to download sources
```bash
mkdir ~/android/cm10-gnex
mkdir ~/bin
cd ~/android/cm10-gnex/
curl https://dl-ssl.google.com/dl/googlesource/git-repo/repo > ~/bin/repo
chmod a+x ~/bin/repo
repo init -u git://github.com/CyanogenMod/android.git -b cm-10.1
```

### Finish setting up repo
```bash
wget -O .repo/local_manifest/local_manifest.xml https://raw.github.com/tiny4579/android_device_samsung_tuna/cm-10.1-linaro/Manifest/local_manifest.xml
```

### Download the source
```bash
cd ~/android/cm10-gnex
repo sync -j16
```
NOTE: This WILL take a long time.

### Build
Make sure we're in ~/android/cm10-gnex...
```bash
cd ~/android/cm10-gnex
```

### List of cherry-picks used.
```bash

#Note: Battery bar doesn't cherry-pick cleanly.  Skip this unless you are able to fix it.

#framework: Battery bar (1/2)
#http://review.cyanogenmod.org/#/c/31912/
cd ~/android/cm10-gnex/frameworks/base
git fetch http://review.cyanogenmod.org/CyanogenMod/android_frameworks_base refs/changes/12/31912/3 && git cherry-pick FETCH_HEAD
#Settings: Battery bar (2/2)
#http://review.cyanogenmod.org/#/c/31913/
cd ~/android/cm10-gnex/packages/apps/Settings
git fetch http://review.cyanogenmod.org/CyanogenMod/android_packages_apps_Settings refs/changes/13/31913/9 && git cherry-pick FETCH_HEAD

#dalvik memory leak fixes/updates
cd ~/android/cm10-gnex/dalvik
#Fix a minor leak in handleVM_CreateString
#https://android-review.googlesource.com/#/c/55023/
git fetch https://android.googlesource.com/platform/dalvik refs/changes/23/55023/1 && git cherry-pick FETCH_HEAD
#Fix a minor leaks caused by failed initializations.(typo)
#https://android-review.googlesource.com/#/c/55130/
git fetch https://android.googlesource.com/platform/dalvik refs/changes/30/55130/1 && git cherry-pick FETCH_HEAD
#Fix a leak in Dalvik_dalvik_system_DexFile_openDexFile
#https://android-review.googlesource.com/#/c/54985/
git fetch https://android.googlesource.com/platform/dalvik refs/changes/85/54985/1 && git cherry-pick FETCH_HEAD
#Tiny optimization for complier templates for arm.
#https://android-review.googlesource.com/#/c/55129/
git fetch https://android.googlesource.com/platform/dalvik refs/changes/29/55129/1 && git cherry-pick FETCH_HEAD
```

### Optional: Linaro building
First, you'll want to pull in these commits
```bash
# Linaro
cd ~/android/cm10-gnex/libcore
git fetch http://review.cyanogenmod.org/CyanogenMod/android_libcore refs/changes/88/31388/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/frameworks/native
git fetch http://review.cyanogenmod.org/CyanogenMod/android_frameworks_native refs/changes/91/31391/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/frameworks/wilhelm
git fetch http://review.cyanogenmod.org/CyanogenMod/android_frameworks_wilhelm refs/changes/90/31390/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/frameworks/ex
git fetch http://review.cyanogenmod.org/CyanogenMod/android_frameworks_ex refs/changes/92/31392/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/frameworks/base
git fetch http://review.cyanogenmod.org/CyanogenMod/android_frameworks_base refs/changes/93/31393/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/frameworks/av
git fetch http://review.cyanogenmod.org/CyanogenMod/android_frameworks_av refs/changes/94/31394/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/webkit
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_webkit refs/changes/95/31395/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/skia
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_skia refs/changes/98/31398/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/stlport
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_stlport refs/changes/97/31397/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/openvpn
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_openvpn refs/changes/99/31399/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/openssl
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_openssl refs/changes/00/31400/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/openssh
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_openssh refs/changes/01/31401/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/lsof
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_lsof refs/changes/02/31402/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/e2fsprogs
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_e2fsprogs refs/changes/03/31403/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/dnsmasq
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_dnsmasq refs/changes/04/31404/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/chromium
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_chromium refs/changes/05/31405/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/bluetooth/bluedroid
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_bluetooth_bluedroid refs/changes/07/31407/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/v8
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_v8 refs/changes/96/31396/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/ping
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_ping refs/changes/09/31409/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/ping6
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_ping6 refs/changes/10/31410/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/system/security
git fetch http://review.cyanogenmod.org/CyanogenMod/android_system_security refs/changes/11/31411/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/external/wpa_supplicant_8
git fetch http://review.cyanogenmod.org/CyanogenMod/android_external_wpa_supplicant_8 refs/changes/12/31412/1 && git cherry-pick FETCH_HEAD
cd ~/android/cm10-gnex/frameworks/rs
git fetch http://review.cyanogenmod.org/CyanogenMod/android_frameworks_rs refs/changes/15/31415/1 && git cherry-pick FETCH_HEAD
```
Then set the environment variables as shown below
```bash
export USE_LINARO_COMPILER_FLAGS=yes
export ANDROID_EABI_TOOLCHAIN_DIR=linaro-4.7
export ARM_EABI_TOOLCHAIN_DIR=linaro-4.7
export DEBUG_NO_STRICT_ALIASING=yes
```
You will also need to pull the Linaro specific local_manifest (This requires a recent version of repo)
```bash
wget -O .repo/local_manifest/linaro_optimizations.xml https://raw.github.com/tiny4579/android_device_samsung_tuna/cm-10.1-linaro/Manifest/linaro_optimizations.xml
```
The above assumes you have the Linaro toolchain extracted or symlinked to ~/android/cm10-gnex/prebuilts/gcc/linux-x86/arm/linaro-4.7

Whether you're using the optional cherry-picks and/or Linaro, pull in the prebuilts...
```bash
./vendor/cm/get-prebuilts
```

And build!
```bash
. build/envsetup.sh && time brunch toro
```
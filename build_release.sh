#!/bin/bash
set -m

# Build script for JBX-Kernel RELEASE
echo "Cleaning out kernel source directory..."
make mrproper
make ARCH=arm distclean

# We build the kernel and its modules first
# Launch execute script in background
# First get tags in shell
echo "Cleaning out Android source directory..."
cd ~/android/system
make ARCH=arm distclean
make mrproper
source build/envsetup.sh
lunch 12

# built kernel & modules
echo "Building modules..."
make -j8 TARGET_KERNEL_SOURCE=/home/mnl-manz/razr_kdev_kernel/android_kernel_motorola_omap4-common/ TARGET_KERNEL_CONFIG=mapphone_OCE_defconfig CONFIG_APANIC_PLABEL="mmcblk1p20" $OUT/boot.img

# We don't use the kernel but the modules
echo "Copying modules to package folder"
cp -r /home/mnl-manz/android/system/out/target/product/spyder/system/lib/modules/* /home/mnl-manz/razr_kdev_kernel/built/rls/system/lib/modules/

# Switch to kernel folder
echo "Entering kernel source..."
cd ~/razr_kdev_kernel/android_kernel_motorola_omap4-common

# Exporting the toolchain (You may change this to your local toolchain location)
echo "Starting configuration of kernel..."
export PATH=~/build/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin:$PATH

# export the flags (leave this alone)
export ARCH=arm
export SUBARCH=arm
export CROSS_COMPILE=arm-eabi-

# define the defconfig (Do not change)
make ARCH=arm mapphone_OCE_defconfig
export LOCALVERSION="-JBX-0.7c-Hybrid"

# execute build command with "-j4 core flag" 
# (You may change this to the count of your CPU.
# Don't set it too high or it will result in a non-bootable kernel.
echo "Building..."
make -j4

# Copy and rename the zImage into nightly/nightly package folder
# Keep in mind that we assume that the modules were already built and are in place
# So we just copy and rename, then pack to zip including the date
echo "Packaging flashable Zip file..."
cp arch/arm/boot/zImage /home/mnl-manz/razr_kdev_kernel/built/rls/system/etc/kexec/kernel

cd /home/mnl-manz/razr_kdev_kernel/built/rls
zip -r "JBX-Kernel-0.7c-Hybrid_$(date +"%Y-%m-%d").zip" *
mv "JBX-Kernel-0.7c-Hybrid_$(date +"%Y-%m-%d").zip" /home/mnl-manz/razr_kdev_kernel/built

# Exporting changelog to file
echo "Exporting changelog to file: '/built/Changelog-[date]'"
cd /home/mnl-manz/razr_kdev_kernel/android_kernel_motorola_omap4-common
git log --oneline --after="yesterday" > /home/mnl-manz/razr_kdev_kernel/android_kernel_motorola_omap4-common/changelog/Changelog_$(date +"%Y-%m-%d")
git add changelog/ .
git commit -m "Added todays changelog"
git push origin JBX

echo "done"

#!/bin/bash
set -m

# Build script for JBX-Kernel NIGHTLY
make mrproper
make ARCH=arm distclean

# We build the kernel and its modules first
# Launch execute script in background
# First get tags in shell
cd ~/android/system
make ARCH=arm distclean
make mrproper
source build/envsetup.sh
lunch 9

# built kernel & modules
make -j8 TARGET_BOOTLOADER_BOARD_NAME=edison TARGET_KERNEL_SOURCE=/home/mnl-manz/razr_kdev_kernel/android_kernel_motorola_omap4-common/ TARGET_KERNEL_CONFIG=mapphone_OCEdison_defconfig BOARD_KERNEL_CMDLINE='root=/dev/ram0 rw mem=1023M@0x80000000 console=null vram=10300K omapfb.vram=0:8256K,1:4K,2:2040K init=/init ip=off mmcparts=mmcblk1:p7(pds),p8(utags),p15(boot),p16(recovery),p17(cdrom),p18(misc),p19(cid),p20(kpanic),p21(system),p22(cache),p23(preinstall),p24(webtop),p25(userdata) mot_sst=1 androidboot.bootloader=0x0A73' BOARD_KERNEL_BASE=0x80000000 BOARD_PAGE_SIZE=0x4096 $OUT/boot.img

# We don't use the kernel but the modules
cp out/target/product/umts_spyder/system/lib/modules/* ~/razr_kdev_kernel/android_kernel_motorola_omap4-common/built/nightly/system/lib/modules/

# Switch to kernel folder
cd ~/razr_kdev_kernel/android_kernel_motorola_omap4-common

# Exporting the toolchain (You may change this to your local toolchain location)
export PATH=~/build/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin:$PATH

# export the flags (leave this alone)
export ARCH=arm
export SUBARCH=arm
export CROSS_COMPILE=arm-eabi-

# define the defconfig (Do not change)
make ARCH=arm mapphone_OCEdison_defconfig
export LOCALVERSION="-JBX-0.6c-Hybrid-Edison"

# execute build command with "-j4 core flag" 
# (You may change this to the count of your CPU.
# Don't set it too high or it will result in a non-bootable kernel.
make -j4

# Copy and rename the zImage into nightly/nightly package folder
# Keep in mind that we assume that the modules were already built and are in place
# So we just copy and rename, then pack to zip including the date
cp arch/arm/boot/zImage built/nightly/system/etc/kexec/kernel
cd built/nightly
zip -r "JBX-Kernel-Hybrid-Edison_$(date +"%Y-%m-%d").zip" *
cp "JBX-Kernel-Hybrid-Edison_$(date +"%Y-%m-%d").zip" ~/razr_kdev_kernel/android_kernel_motorola_omap4-common/built


# Cleaning out
rm ~/razr_kdev_kernel/android_kernel_motorola_omap4-common/built/nightly/system/etc/kexec/*
rm ~/razr_kdev_kernel/android_kernel_motorola_omap4-common/built/rls/system/etc/kexec/*
rm ~/razr_kdev_kernel/android_kernel_motorola_omap4-common/built/nightly/system/lib/modules/*
rm ~/razr_kdev_kernel/android_kernel_motorola_omap4-common/built/nightly/**
rm ~/razr_kdev_kernel/android_kernel_motorola_omap4-common/built/rls/*

echo "done"

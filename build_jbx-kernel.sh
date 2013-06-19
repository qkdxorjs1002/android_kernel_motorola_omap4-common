#!/bin/bash

# Build script for JBX-Kernel

# Clean out prior builds
make clean

# Exporting the toolchain (You may change this to your local toolchain location)
export PATH=~/build/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin:$PATH

# export the flags (leave this alone)
export ARCH=arm
export SUBARCH=arm
export CROSS_COMPILE=arm-eabi-

# define the defconfig (Do not change)
make ARCH=arm mapphone_OCE_defconfig
export LOCALVERSION="-JBX-0.6c-Hybrid"

# execute build command with "-j4 core flag" 
# (You may change this to the count of your CPU.
# Don't set it too high or it will result in a non-bootable kernel.

make -j4

# Copy and rename the zImage into rls/nightly package folder
# Keep in mind that we assume that the modules were already built and are in place
# So we just copy and rename

echo Have you built the modules? Otherwise press STRG+C

while true; do
    read -p "Is this a release? 'No' will handle as nightly (y/n)" yn
    case $yn in
        [Yy]* ) cp arch/arm/boot/zImage built/rls/system/etc/kexec/kernel; cd built/rls; zip -r "JBX-Kernel-Hybrid_$(date +"%Y-%m-%d").zip" *"; break;;
        [Nn]* ) cp arch/arm/boot/zImage built/nightly/system/etc/kexec/kernel; cd built/nightly; zip -r "JBX-Kernel-Hybrid_NITHLY_$(date +"%Y-%m-%d").zip" *"; exit;;
        * ) echo "Please answer yes or no.";;
    esac
done

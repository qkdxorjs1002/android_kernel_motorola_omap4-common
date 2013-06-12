#!/bin/sh

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
export LOCALVERSION="-JBX-0.6a-Hybrid"

# execute build command with "-j4 core flag" 
# (You may change this to the count of your CPU.
# Don't set it too high or it will result in a non-bootable kernel.

make -j4

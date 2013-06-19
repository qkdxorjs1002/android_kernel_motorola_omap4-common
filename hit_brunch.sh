#!/bin/sh

# Build script for JBX-Kernel

# We build the kernel and its modules first
cd ~/android/system
make ARCH=arm distclean
source build/envsetup.sh
brunch umts_spyder

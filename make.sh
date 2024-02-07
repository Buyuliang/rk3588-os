#!/bin/bash
#2022.01.24 by Y

# compile ================================================================
make rk3588tpl

if [ $? -ne 0 ]; then
    echo -e "\e[91m compile failed!"
	exit
fi

# makeRKLoader_With_Tpl.sh ===============================================
dd if=/dev/zero of=fakeusbplug.bin count=1 bs=1
dd if=/dev/zero of=fakespl.bin     count=1 bs=1

sudo ./RKLoaderTools/boot_merger RKLoader.ini

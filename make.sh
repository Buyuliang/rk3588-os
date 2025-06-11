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


/home/tom/project/littleSystem/build/uboot/build/tools/mkimage -n rk3588 -T rksd -d /home/tom/project/littleSystem/build/rkbin/bin/rk35/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin:tpl.bin idbloader.img
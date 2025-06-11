#! /bin/bash

make clean && make

../tools/mkimage -n rk3588 -T rksd -d ../tools/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin:spl.bin idbloader.img
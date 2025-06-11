# PREFIX=/mnt/e/Dev/EE/Rockchip/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
# PREFIX=/home/tom/project/tools/prebuilts/rk3588-blade3-sdk-rk-prebuilts-gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu-master/bin/aarch64-none-linux-gnu-
PREFIX=aarch64-linux-gnu-
CC=$(PREFIX)gcc
LD=$(PREFIX)ld
AR=$(PREFIX)ar
OBJCOPY=$(PREFIX)objcopy
OBJDUMP=$(PREFIX)objdump

# Add GCC lib
# PLATFORM_LIBS += -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc

rk3588tpl : rk3588tpl_start.S  main.c
	$(CC) -D__ASSEMBLY__ -I. -DCONFIG_ARM64 -nostdinc -fno-builtin -mstrict-align -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -fno-common -ffixed-x18 -pipe -march=armv8-a+nosimd -g -c -o rk3588tpl_start.o  rk3588tpl_start.S

	$(CC) -nostdlib -fno-builtin -g -c -o main.o  main.c

	$(LD) -Trk3588tpl.lds -g rk3588tpl_start.o main.o -o out_elf  $(PLATFORM_LIBS)
	
	$(OBJCOPY) -O binary -S out_elf tpl.bin
	#$(OBJDUMP) -D -m aarch64  out_elf > out.dis
	#/mnt/e/Dev/EE/Rockchip/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-objdump -D -b binary -m aarch64 bootrom.bin > out.dis

clean:
	rm -f out.dis  tpl.bin out_elf  *.o *.bin

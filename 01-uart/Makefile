PREFIX := aarch64-linux-gnu-
CC := $(PREFIX)gcc
LD := $(PREFIX)ld
AR := $(PREFIX)ar
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump

# 编译选项
CFLAGS := -D__ASSEMBLY__ -I. -DCONFIG_ARM64 -nostdinc -fno-builtin \
          -mstrict-align -ffunction-sections -fdata-sections -fno-common \
          -ffixed-r9 -ffixed-x18 -pipe -march=armv8-a+nosimd -g

# 链接选项
LDFLAGS := -Tspl.lds -nostdlib -g

# 目标文件
OBJS := start.o

# 默认目标
all: spl.bin

# 编译汇编文件
start.o: start.s
	$(CC) $(CFLAGS) -c -o $@ $<

# 链接生成ELF
spl.elf: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

# 转换为二进制
spl.bin: spl.elf
	$(OBJCOPY) -O binary -S $< $@

# 生成反汇编（可选）
dis: spl.elf
	$(OBJDUMP) -D -m aarch64 $< > spl.dis

# 清理规则
clean:
	rm -f *.o *.elf *.bin *.dis *.img

# 伪目标
.PHONY: all clean dis
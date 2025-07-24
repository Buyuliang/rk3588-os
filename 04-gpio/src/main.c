// main.c

#include "printf.h"
#include "common.h"


extern void tom_test();

void main() {
    printf("Booting...\n");
    printf("Hello %s, num = %d, hex = %x, char = %c\n", "world", 123, 0xABCD, 'Z');
    io_test();
    //GICv3_Demo();
}


void io_test() {
    printf("io_test...\n");
u32 BUS_IOC = 0xFD5F8000;
u32 BUS_IOC_GPIO4A_IOMUX_SEL_H = 0x0084;
    rk_clrsetreg(BUS_IOC + BUS_IOC_GPIO4A_IOMUX_SEL_H, 0x3 << 6, 0x0 << 6); // 把 GPIO4_A3 的复用设为 GPIO 模式

u32 GPIO4 = 0xFEC50000;
u32 GPIO_SWPORT_DDR_L = 0x0008;
    rk_setreg(GPIO4 + GPIO_SWPORT_DDR_L, (1 << 3)); //设置 GPIO4_A3 为 输出 功能

u32 GPIO_SWPORT_DR_L = 0x0000;
    rk_setreg(GPIO4 + GPIO_SWPORT_DR_L, (1 << 3)); //设置 GPIO4_A3 为 HIGH
    // rk_setreg(GPIO4 + GPIO_SWPORT_DR_L, (0 << 3)); //设置 GPIO4_A3 为 LOW
}
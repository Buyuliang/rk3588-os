// main.c

#include "printf.h"
#include "common.h"


extern void tom_test();
extern void sleep(u32 time);

void main() {
    printf("Booting...\n");
    printf("Hello %s, num = %d, hex = %x, char = %c\n", "world", 123, 0xABCD, 'Z');
    io_test();
    //GICv3_Demo();
}


void io_test() {
    printf("io_test...\n");
u32 BUS_IOC = 0xFD5F8000;
u32 BUS_IOC_GPIO4A_IOMUX_SEL_L = 0x0080;
    rk_clrsetreg(BUS_IOC + BUS_IOC_GPIO4A_IOMUX_SEL_L, 0xF << 12, 0x0 << 12); // 把 GPIO4_A3 的复用设为 GPIO 模式

u32 BUS_IOC_GPIO1B_IOMUX_SEL_L = 0x0028;
    rk_clrsetreg(BUS_IOC + BUS_IOC_GPIO1B_IOMUX_SEL_L, 0xF << 8, 0x0 << 8); // 把 GPIO1_B2 的复用设为 GPIO 模式

u32 GPIO4 = 0xFEC50000;
u32 GPIO_SWPORT_DDR_L = 0x0008;
    rk_setreg(GPIO4 + GPIO_SWPORT_DDR_L, (1 << 3)); //设置 GPIO4_A3 为 输出 功能

u32 GPIO1 = 0xFEC20000;
    rk_setreg(GPIO1 + GPIO_SWPORT_DDR_L, (1 << 10)); //设置 GPIO1_B2 为 输出 功能
    
u32 GPIO_SWPORT_DR_L = 0x0000;
    rk_setreg(GPIO4 + GPIO_SWPORT_DR_L, (1 << 3)); //设置 GPIO4_A3 为 HIGH
    // rk_setreg(GPIO4 + GPIO_SWPORT_DR_L, (0 << 3)); //设置 GPIO4_A3 为 LOW

    while (1) {
        printf("io_test out HIGH\n");
        writel((1 << 10) << 16 | (1 << 10), GPIO1 + GPIO_SWPORT_DR_L); //设置 GPIO1_B2 为 HIGH
        sleep(1000); // 延时1秒
        printf("io_test out LOW\n");
        writel((1 << 10) << 16 | (0 << 10), GPIO1 + GPIO_SWPORT_DR_L); //设置 GPIO1_B2 为 LOW
        sleep(1000);
    }
}

void sleep(u32 time) {
    while (time--) {
        for (int i = 0; i < 1000; i++);
            for (int j = 0; j < 1000; j++);
    }
}

#include "common.h"
#include "gicv3.h"
#include "printf.h"


/**********************************************************/
/*************** void GICInit() 是中断主函数 **************/
/**********************************************************/

static GICD gicd;



//{ 第一个使用 GICv3 PPI 中断的设备, TEE侧的timer0
#define BASE_FREQUENCY		24000000
#define STIMER0_BASE		0xFF710000
	#define TIMER_LOADR		0x0000
	#define TIMER_CURRR		0x0008
	#define TIMER_CTRLR		0x0010
	#define TIMER_ISTTR		0x0018

void Init_rk3568_Timer()
{
	u64 value;// 	writeq(value, (u64*)(STIMER0_BASE + TIMER_CTRLR));

	
	value = 0x1;		// 要先禁用这个定时器，才能对它进行配置
	value&= ~(1<<1);	// timer working in free-running MODE
	value|= (1<<2);		// 允许这个定时器触发中断
	writeq(value, (u64*)(STIMER0_BASE + TIMER_CTRLR));

	// 配置 load value
	value = BASE_FREQUENCY;		/*	timer 会这样工作：
									来一个脉冲计一次数，所以经过 BASE_FREQUENCY 个脉冲后，就过去一秒钟了
								 */
	writeq(value, (u64*)(STIMER0_BASE + TIMER_LOADR));
	
	// 开启定时器
	value|= (1<<0);		// enable this timer
	value&= ~(1<<1);	// timer working in free-running MODE
	value|= (1<<2);		// 允许这个定时器触发中断
	writeq(value, (u64*)(STIMER0_BASE + TIMER_CTRLR));



}

#define GenCounterFreq  24000000 //0x5F5E100   					//100MHz
#define TICK_CYCLES     (2000*(GenCounterFreq/1000))	//计数够2s时触发一次中断
void setGenTimerFreq(s32 freq);
u64 getPhyCount();
void setEL3PhyTimerCV(u64 cv);
void setEL3PhyTimerCtrl(u32 ctl);
void Init_qemu_Timer()
{
	setGenTimerFreq(GenCounterFreq);
	setEL3PhyTimerCV(getPhyCount()+TICK_CYCLES);
	setEL3PhyTimerCtrl(0x01);
}
void vApplicationFIQHandler(u64 intID)
{
	u64 count;
	count = getPhyCount();
	setEL3PhyTimerCV(count+TICK_CYCLES);
	printf("INT %d, Current Generic Timer Count: %d\n", (s32)intID, count);
}

void InitTestTimer()
{
GICv3Interrupt TeeTimerInt;

#ifdef PLAT_QEMU
	Init_qemu_Timer();

	// 29号中断(29号中断是 TEE 环境的 timer 中断，根据中断号划分，29号属于 PPI 中断，即独享的中断号)
	TeeTimerInt.intr_num = 29;
#else
	Init_qemu_Timer();
	//Init_rk3568_Timer();

	// 29号中断(29号中断是 TEE 环境的 timer 中断，根据中断号划分，29号属于 PPI 中断，即独享的中断号)
	TeeTimerInt.intr_num = 29;
	//TeeTimerInt.intr_num = 139;
#endif

	TeeTimerInt.intr_grp = IntGroupG0;
	TeeTimerInt.intr_pri = 0;
	TeeTimerInt.intr_cfg = 0;
	TeeTimerInt.isEnable = 1;
	GICv3_InterruptConfig(&TeeTimerInt);
}
//}



void arch_cpu_init()
{

#define PMUGRF_BASE		0xfdc20000
#define GRF_BASE		0xfdc60000
#define GRF_GPIO1B_IOMUX_H	0x0C
#define GRF_GPIO1C_IOMUX_L	0x10
#define GRF_GPIO1C_IOMUX_H	0x14
#define GRF_GPIO1D_IOMUX_L	0x18
#define GRF_GPIO1D_IOMUX_H	0x1C
#define GRF_GPIO1B_DS_2		0x218
#define GRF_GPIO1B_DS_3		0x21c
#define GRF_GPIO1C_DS_0		0x220
#define GRF_GPIO1C_DS_1		0x224
#define GRF_GPIO1C_DS_2		0x228
#define GRF_GPIO1C_DS_3		0x22c
#define GRF_GPIO1D_DS_0		0x230
#define GRF_GPIO1D_DS_1		0x234
#define GRF_GPIO1D_DS_2		0x238
#define GRF_SOC_CON4		0x510
#define EDP_PHY_GRF_BASE	0xfdcb0000
#define EDP_PHY_GRF_CON0	(EDP_PHY_GRF_BASE + 0x00)
#define EDP_PHY_GRF_CON10	(EDP_PHY_GRF_BASE + 0x28)
#define PMU_BASE_ADDR		0xfdd90000
#define PMU_NOC_AUTO_CON0	(0x70)
#define PMU_NOC_AUTO_CON1	(0x74)
#define CRU_BASE		0xfdd20000
#define CRU_SOFTRST_CON26	0x468
#define CRU_SOFTRST_CON28	0x470
#define SGRF_BASE		0xFDD18000
#define SGRF_SOC_CON3		0xC
#define SGRF_SOC_CON4		0x10
#define PMUGRF_SOC_CON15	0xfdc20100
#define CPU_GRF_BASE		0xfdc30000
#define GRF_CORE_PVTPLL_CON0	(0x10)
#define USBPHY_U3_GRF		0xfdca0000
#define USBPHY_U3_GRF_CON1	(USBPHY_U3_GRF + 0x04)
#define USBPHY_U2_GRF		0xfdca8000
#define USBPHY_U2_GRF_CON0	(USBPHY_U2_GRF + 0x00)
#define USBPHY_U2_GRF_CON1	(USBPHY_U2_GRF + 0x04)

#define PMU_PWR_GATE_SFTCON	(0xA0)
#define PMU_PWR_DWN_ST		(0x98)
#define PMU_BUS_IDLE_SFTCON0	(0x50)
#define PMU_BUS_IDLE_ST		(0x68)
#define PMU_BUS_IDLE_ACK	(0x60)

#define EBC_PRIORITY_REG	(0xfe158008)

	/*
	 * When perform idle operation, corresponding clock can
	 * be opened or gated automatically.
	 */
	writel(0xffffffff, PMU_BASE_ADDR + PMU_NOC_AUTO_CON0);
	writel(0x000f000f, PMU_BASE_ADDR + PMU_NOC_AUTO_CON1);

	/* Set the emmc sdmmc0 to secure */
	writel(((0x3 << 11 | 0x1 << 4) << 16), SGRF_BASE + SGRF_SOC_CON4); // 有的
	/* set the emmc ds to level 2 */
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1B_DS_2);
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1B_DS_3);
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1C_DS_0);
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1C_DS_1);
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1C_DS_2);
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1C_DS_3); // 有的

#if defined(CONFIG_ROCKCHIP_SFC)
	/* Set the fspi to secure */
	writel(((0x1 << 14) << 16) | (0x0 << 14), SGRF_BASE + SGRF_SOC_CON3); // 有的
#endif


	/* Disable eDP phy by default */
	writel(0x00070007, EDP_PHY_GRF_CON10);
	writel(0x0ff10ff1, EDP_PHY_GRF_CON0);

	/* Set core pvtpll ring length */
	writel(0x00ff002b, CPU_GRF_BASE + GRF_CORE_PVTPLL_CON0); // 有的

	/*
	 * Assert reset the pipephy0, pipephy1 and pipephy2,
	 * and de-assert reset them in Kernel combphy driver.
	 */
	 writel(0x02a002a0, CRU_BASE + CRU_SOFTRST_CON28); // 有的

	 /*
	  * Set USB 2.0 PHY0 port1 and PHY1 port0 and port1
	  * enter suspend mode to to save power. And USB 2.0
	  * PHY0 port0 for OTG interface still in normal mode.
	  */
	 writel(0x01ff01d1, USBPHY_U3_GRF_CON1);
	 writel(0x01ff01d1, USBPHY_U2_GRF_CON0);
	 writel(0x01ff01d1, USBPHY_U2_GRF_CON1);

}

void rockchip_stimer_init(void)
{
#define COUNTER_FREQUENCY			24000000
#define CONFIG_ROCKCHIP_STIMER_BASE 0xfdd1c020

	asm volatile("msr CNTFRQ_EL0, %0"
		     : : "r" (COUNTER_FREQUENCY));

	writel(0, CONFIG_ROCKCHIP_STIMER_BASE + 0x10);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 4);
	writel(1, CONFIG_ROCKCHIP_STIMER_BASE + 0x10);
}


// 检查配置是否正确!
void CheckConfig()
{
	u64 addr;
	u32 value;
	u64 Current_GICR_BASE;
	s32 i;
	s32 maxIntID = 31;
	
	printf("[CHK] daif: %032b\r\n", read_daif());
	
	
	addr = GICD_BASE + GICD_CTLR;
	printf("[CHK] GICD_BASE + GICD_CTLR: %032b\r\n", readl(addr));

	Current_GICR_BASE = get_RD_Base(0);
	
	addr = Current_GICR_BASE + GICR_CTLR;
	printf("[CHK] Current_GICR_BASE + GICR_CTLR: %032b\r\n", readl(addr));
	
	
	addr = Current_GICR_BASE + GICR_IGRPMODR0;
	printf("[CHK] Current_GICR_BASE + GICR_IGRPMODR0: %032b\r\n", readl(addr));
	addr = Current_GICR_BASE + GICR_IGROUPR0;
	printf("[CHK] Current_GICR_BASE + GICR_IGROUPR0: %032b\r\n", readl(addr));

	// GICR_ISENABLER0
	addr = Current_GICR_BASE + GICR_ISENABLER0;
	printf("[CHK] Current_GICR_BASE + GICR_ISENABLER0: %032b\r\n", readl(addr));
	
	addr = Current_GICR_BASE + GICR_ICENABLER0;
	printf("[CHK] Current_GICR_BASE + GICR_ICENABLER0: %032b\r\n", readl(addr));

	// GICD_ISENABLE
	for(i=0;i<10;i++)
	{
		addr = GICD_BASE + GICD_ISENABLE + (4*i);
		printf("[CHK] GICD_BASE + GICD_ISENABLE[%d]: %032b\r\n", i, readl(addr));
		addr = GICD_BASE + GICD_ICENABLE + (4*i);
		printf("[CHK] GICD_BASE + GICD_ICENABLE[%d]: %032b\r\n", i, readl(addr));
	}
	
	// GICD_IROUTERn
	for(i=130;i<140;i++)
	{
		addr = GICD_BASE + GICD_IROUTERn + (8*i);
		printf("[CHK] GICD_BASE + GICD_IROUTERn[%d]: %016x\r\n", i, readq(addr));
	}



	addr = Current_GICR_BASE + GICR_IPRIORITYR;
	for(i=0; i<= maxIntID; i+=4)
	{
		printf("[CHK] Current_GICR_BASE + GICR_IPRIORITYR: %032b\r\n", readl(addr));
		addr += 4;
	}
	
	addr = Current_GICR_BASE + GICR_ICFGR0;
	for(i=0; i<= maxIntID; i+=16)
	{
		printf("[CHK] Current_GICR_BASE + GICR_ICFGR0: %032b\r\n", readl(addr));
		addr += 4;
	}

	// 
	addr = GICD_BASE + GICD_IPRIORITYR;
	for(i=0; i<= maxIntID; i+=4)
	{
		printf("[CHK] GICD_BASE + GICD_IPRIORITYR: %032b\r\n", readl(addr));
		addr += 4;
	}
	
	addr = GICD_BASE + GICD_ICFGR;
	for(i=0; i<= maxIntID; i+=16)
	{
		printf("[CHK] GICD_BASE + GICD_ICFGR: %032b\r\n", readl(addr));
		addr += 4;
	}
}

void GICv3_Demo()
{
	printf("\r\n GICv3_Demo()\n");
	
	//rockchip_stimer_init();
	//arch_cpu_init();

	/* 第一步，硬件虚拟机划分，配置为：
	 * 1). 不使用REE，没有Group1_NS
	 * 2). 只使用 (EL3 SecureMonitor)，即只有EL3层的代码
	 */
	printf(" TEE_REE_Setup\n");
	TEE_REE_Setup();


	/* 第二步，GICv3 初始化
	 */
	printf(" GICv3Init\n");
	GICv3Init();
	
	
	/* 第三步，配置 timer 中断
	 */
	InitTestTimer();

	// 试着产生一个 smc 中断，让它触发 SYNC 中断，若能正常触发说明vectors设置正确了，当然，这个无关GIC的事。
	asm volatile ("smc #0" : : : "memory");


	// // 最后改一次
	// u32* addr;
	// u32 value;
	// addr = (u32*)(GICD_BASE + GICD_CTLR);
	// value = 0x35; // 这个可以
	// writel(value, addr);
	// ctrl_Dist_WaitReadWriteFinished();

	// asm volatile("msr daifclr, #0xf");

	//CheckConfig();
}


// 中断初始化函数
s32 GICv3Init()
{
	printf(" RK3568 GIC600 mini GICv3_Driver in EL3\n");

	Need(GICD_BASE != 0U);
	Need(GICR_BASE != 0U);
	Need(get_CurrentEL() == 3);
	
	GICD* pGicd = &gicd;


	//{ 第一步，读取基本信息
	printf(" reading GIC info\n");
	gicd.gicdBase = GICD_BASE;
	gicd.gicrBase = GICR_BASE;
	gicd.MAX_RD_COUNT = 6;
	//gicd.nProcessor = 4;
	gicd.gic_modelID = get_RD_modelID(0);
	gicd.gic_version = get_gic_version(pGicd);
	gicd.nSPI= get_Max_SPI_IntID(pGicd);
	gicd.CurrPE_Affinity = get_CurrPE_Affinity();
	gicd.nReDistributor = get_RD_Count();
	gicd.CurrRD = get_CurrRD();
	gicd.GICv3_SRE = read_ICC_SRE_EL3();

	//{ Dump GICv3 Info
	u32* addr;
	u32 value = 0;
	u32 Current_GICR_BASE = GICR_BASE;
	s32 i=0;

	#if Use_InfoDump
	printf("\r\n GICv3DumpInfo()\r\n");
	printf(" gicd.gic_modelID    : 0x%08x%s\r\n", gicd.gic_modelID, ((gicd.gic_modelID == IIDR_MODEL_ARM_GIC_600)?"(GIC600)":""));
	printf(" gicd.gic_version    : 0x%08x\r\n", gicd.gic_version);
	printf(" gicd.nSPI           : %d\r\n", gicd.nSPI);
	printf(" gicd.CurrPE_Affinity: 0x%08x\r\n", gicd.CurrPE_Affinity);
	printf(" gicd.nReDistributor : 0x%08x\r\n", gicd.nReDistributor);
	printf(" gicd.CurrRD         : 0x%08x\r\n", gicd.CurrRD);
	printf(" gicd.GICv3_SRE      : 0x%08x\r\n", gicd.GICv3_SRE);						// 重启时 SRE 为1，表示使用寄存器方式配置cpuif (GICv3默认是这种方式，不可改为0!)
																						//        SRE 为0，表示MMIO方式配置cpuif。(为了兼容GICv2才会重启时才会为0，也即使用MMIO方式)
	printf(" GICD_CTLR           : 0x%08x\r\n", readl((u32*)(GICD_BASE + GICD_CTLR)));	// 重启时若 ARE_S 为0，表示兼容 GICv2
	printf(" scr_el3             : 0b%064b\r\n", read_scr_el3());						// 


	s32 n = get_RD_Count();
	for(i=0; i< n;i++)
	{
		u64 typeVal = get_RD_TypeValue(i);
		printf("\r\n ReDistributor%d, %s\r\n", i, get_RD_IsLastOne_FromTypeValue(typeVal)?"the last one!":"");
		printf("\ttarget_PE_Number: 0x%08x\r\n", get_RD_ProcessorID_FromTypeValue(typeVal));
		printf("\tAffinity_Address: 0x%08x\r\n", get_RD_Affinity_FromTypeValue(typeVal));

		u64 Current_GICR_BASE = get_RD_Base(i);
		u32* addr = (u32*)(u64)(Current_GICR_BASE + GICR_WAKER);
		u32 value = readl(addr);
		printf("\tGICR_BASE       : 0x%08x\r\n", Current_GICR_BASE);
		printf("\tGICR_CTLR       : 0x%08x\r\n", readl((u32*)(u64)(Current_GICR_BASE + GICR_CTLR)));
		printf("\tGICR_BASE.WAKER : 0x%08x\r\n", readl((u32*)(u64)(Current_GICR_BASE + GICR_WAKER)));
		printf("\tGICR_BASE.PWRR  : 0x%08x\r\n", readl((u32*)(u64)(Current_GICR_BASE + GICR_PWRR)));
	}
	printf("\r\n\r\n");
	#endif
	//}


	//}


	/* 第二步，Distributor(Distr) 配置为：
	 * 1). 只允许 G0S 和 G1S 的中断
	 * 2). 启用 ARE_S (允许 secure EE 使用 affinity 路由功能)
	 * 3). 所有 SPI 中断划到 G0S
	 */
	printf("\r\n\r\n GICv3_Config_Distributor\n");
	GICv3_Config_Distributor();
	GICv3_DefaultGrouping_SPI(pGicd, IntGroupG0);
	

	/* 第三步，ReDistributor(RDist)，只配置当前PE对应的 ReDistributor(其它的RD不设置)
	 * 配置为：
	 * 1). 禁用 LPIs
	 * 2). 所有 SGI | PPI 中断划到 G0S
	 */
	printf("\r\n\r\n GICv3_Config_ReDistributor, CurrRD: %d\n", gicd.CurrRD);
	GICv3_Config_ReDistributor(gicd.CurrRD);
	GICv3_DefaultGrouping_SGI_PPI(gicd.CurrRD, IntGroupG0);
	//return -1;

	/* 第四步，CpuInterface(Cpuif)
	 */
	printf("\r\n\r\n GICv3_Config_CPUInterface, CurrRD: %d\n", gicd.CurrRD);
	GICv3_Config_CPUInterface(gicd.CurrRD);
	
	/* 第五步，PE 中打开 daif 寄存器
	 */
	GICv3_Config_PE();
}



//{ TEE REE 硬件资源划分(商用情况下，实际的TEE/REE划分应该是非常复杂的)
extern u64 vectors;
void TEE_REE_Setup()
{
	// vbar_el3 配置
	u64 pVectors = (u64)(&vectors);
	printf(" vectors set to      : 0x%016x\r\n\r\n", pVectors);
	write_vbar_el3(pVectors);

	// scr_el3 配置
	u64 value = read_scr_el3();
	//value |= (1 << 11);	//set ST bit (disable trapping of timer control registers)
	value |= (1 << 10);		//set RW bit (next lower EL in aarch64，下一层EL使用 AArch64)
	value |= (1 << 3);		//Set EA bit (SError routed to EL3)
	value |= (1 << 2);		//Set FIQ bit (FIQs routed to EL3)
	value |= (1 << 1);		//Set IRQ bit (IRQs routed to EL3)
	//value &= (1 << 0);		//Clr NS bit (Current EE in Secure)// 切换了这个状态中断就不会触发！！
	write_scr_el3(value);
	
	write_cptr_el3(0x0);		//允许 EL2、EL1 访问一些特定的寄存器(开权限)，打开SIMD和FP浮点运算功能
	write_cntfrq_el0(24000000);	//晶振频率设置为 24MHz
	isb();
}
//}


//{ Distributor，ReDistributor，CpuInterface，PE 配置
// GIC Distributor 配置
void GICv3_Config_Distributor()
{
	u32* addr;

	//通过 GICD_CTLR 寄存器进行配置
	addr = (u32*)(GICD_BASE + GICD_CTLR);

	//{ 要先清掉 enable 位(禁用中断)，才能在下面设置 ARE_S 位，否则可能会触发 system error !
	u32 value = GICD_READL(GICD_CTLR);
	value &= ~(1 << 0);	//clear EnableG0S
	value &= ~(1 << 1);	//clear EnableG1NS
	value &= ~(1 << 2);	//clear EnableG1S
	writel(value, addr);
	ctrl_Dist_WaitReadWriteFinished();
	//}

	//{ 开启ARE功能，即设置 ARE_S/ARE_NS 位(各组中断都处于禁止状态时才能设置这两个位)
	value = 0;
	value |= (1 << 4);	//Enable  ARE_S
	value &= ~(1 << 5);	//Disable ARE_NS
	writel(value, addr);
	ctrl_Dist_WaitReadWriteFinished();
	//}

	//{ 启用GICD中断
	value |= (1 << 0);	//EnableG0S
	value |= (1 << 1);	//EnableG1NS
	value |= (1 << 2);	//EnableG1S
	value |= (1 << 6);	//DS 位设置为1
						//表示只有G0S和G1S两个分组。
						//注意：When GICD_CTLR.DS == 1, LPIs are always Group 1 interrupts.
	value = 0x37;
	value = 0x35; // 这个可以
	writel(value, addr);
	ctrl_Dist_WaitReadWriteFinished();
	//}
}

// GIC ReDistributor 配置
void GICv3_Config_ReDistributor(s32 RD_Index)
{
	Need( RD_Index >= 0 && RD_Index < gicd.nReDistributor);
	u32* addr;
	u32 value;

	u64 Current_GICR_BASE = get_RD_Base(RD_Index);

	// 唤醒此 RDist
	gicv3_rdistif_on(RD_Index);


	//通过 GICR_CTLR 寄存器进行配置
	addr = (u32*)(u64)(Current_GICR_BASE + GICR_CTLR);
	value = readl(addr);
	
	u32 UWP = (value >> 31) & 0x1; // Upstream Write Pending, read only

	value &= ~(1<<0);  // clear to diable LPIs
	value &= ~(1<<24); // clear DPG0  to enable G0 interrupts
	value |=  (1<<25); // set  DPG1NS to disable G1NS interrupts
	value &= ~(1<<26); // clear DPG1S to enable G1S interrupts
	value = 0; //这个可以
	writel(value, addr);
}

// GIC CPUInterface 配置
void GICv3_Config_CPUInterface(s32 RD_Index)
{
	/* NOTE: ( GICv3_GICv4_Software_Overview_Official_Release_B.pdf 第17页)
	Software can check for GIC System register support by reading ID_AA64PFR0_EL1 for
	the PE, see ARM® Architecture Reference Manual, ARMv8, for ARMv8-A architecture profile for details.
	*/
	Need( (gicd.GICv3_SRE & 0x1) == 1);
	
	// 唤醒自己的 cpuif(即当前PE对应的cpuif)
	ctrl_RD_wakeup_TheConnectedCpuif(RD_Index);


	// 参考：E:\Dev\EE\Rockchip\arm-trusted-firmware-master\drivers\arm\gic\v3\gicv3_main.c 中 gicv3_cpuif_enable 函数
	// 参考：https://developer.arm.com/documentation/ddi0601/2021-12/AArch64-Registers/ICC-SRE-EL3--Interrupt-Controller-System-Register-Enable-register--EL3-?lang=en
#define ICC_SRE_EN_BIT		(1<<3)	// 为1则允许低ELx级别访问 ICC_SRE_EL1 和 ICC_SRE_EL2
									// 为0则低级ELx访问这两寄存器时会中断到El3!
#define ICC_SRE_DIB_BIT		(1<<2)
#define ICC_SRE_DFB_BIT		(1<<1)
#define ICC_SRE_SRE_BIT		(1<<0)	// 为0则要求EL3使用MMIO方式进行 cpuif 配置，此时 ICH_*/ICC* 两类寄存器对EL3层不可见，读写都会触发中断。
									// 为1则 ICH_*/ICC* 两类寄存器对EL3层都可以访问！GICv3要求这个位为1，否则芯片厂商要提供 GICC_BASE 基地址，不然将无法进行 cpuif 配置！

	u32 value;

	/* Disable the legacy interrupt bypass */
	value = ICC_SRE_DIB_BIT | ICC_SRE_DFB_BIT;

	/*
	 * Enable system register access for EL3 and allow lower exception
	 * levels to configure the same for themselves. If the legacy mode is
	 * not supported, the SRE bit is RAO/WI
	 */
	value |= (ICC_SRE_EN_BIT | ICC_SRE_SRE_BIT);
	write_ICC_SRE_EL3(read_ICC_SRE_EL3() | value);

	isb();
	/* Add DSB to ensure visibility of System register writes */



	/* Program the idle priority in the PMR 
	 * Priority Mask register*/
	write_ICC_PMR_EL1((u32)0x0FF);

	/* Enable Group0 interrupts */
	write_ICC_IGRPEN0_EL1((1 << 0));

	/* Enable Group1 Secure interrupts */
	//write_ICC_IGRPEN1_EL3(read_ICC_IGRPEN1_EL3() | (1 << 1));

	// 配置EL3层允许group1中断
	write_ICC_IGRPEN1_EL3(read_ICC_IGRPEN1_EL3() | (0x3));
	
	write_ICC_CTLR_EL3(0x0);
	
	// 配置EL1层允许group1中断，ICC_IGRPEN1_EL1 是Banked的，不过在 EL3 对ICC_IGRPEN1_EL3进行操作，就是对两个安全状态的 ICC_IGRPEN1_EL1 进行操作！
	write_ICC_IGRPEN1_EL1(0x3);
	
	
	isb();

	dsb();

}

// PE 配置
void GICv3_Config_PE()
{
	asm volatile("msr daifclr, #0xf");
	//write_daif(0x0);
}

//}


//{ 逐中断配置(参数为 GICv3Interrupt* ，即中断描述结构)
void GICv3_InterruptConfig(GICv3Interrupt* pInt)
{
	s32 intID = pInt->intr_num;
	if(intID < 32)
	{
		GICv3_SGI_PPI_Config(pInt, gicd.CurrRD);
	}
	else if(intID >= 32 && intID < gicd.nSPI)
	{
		GICv3_SPI_Config(pInt);
	}else{
		// LPI 未支持
		printf("\r\n LPI not supported!\r\n");
	}
}


void GICv3_SGI_PPI_Config(GICv3Interrupt* pInt, s32 RD_Index)
{
	s32 intID = pInt->intr_num;
	Need(intID >= 0 && intID < 32);
	
	
	u64 Current_GICR_BASE = get_RD_Base(RD_Index);
	s32 nPropertyWidth;

	//{ 设置中断分组，一个中断号占用1个bit，因为每次都是对32位寄存器进行操作，每次可以处理32个中断号	
	u32 valueM = 0;
	u32 valueG = 0;
	switch(pInt->intr_grp){
		case IntGroupG0:
			valueM = 0x0; valueG = 0x0;
			break;
		case IntGroupG1S:
			valueM = 0xFFFFFFFF; valueG = 0;
			break;
		case IntGroupG1NS:
		default:
			valueM = 0; valueG = 0xFFFFFFFF;
			break;
	}
	//通过 GICR_IGRPMODR0 和 GICR_IGROUPR0 寄存器进行配置
	nPropertyWidth = 1;
	BitMem32_Copy((u32*)(Current_GICR_BASE + GICR_IGRPMODR0), nPropertyWidth, intID, valueM);
	BitMem32_Copy((u32*)(Current_GICR_BASE + GICR_IGROUPR0),  nPropertyWidth, intID, valueG);
	//}

	//{ 设置中断开关，一个中断号占用1个bit
	// GICR 中要设置
	nPropertyWidth = 1;
	BitMem32_Copy((u32*)(Current_GICR_BASE + GICR_ISENABLER0), nPropertyWidth, intID, pInt->isEnable);
	//ctrl_Dist_WaitReadWriteFinished();
	/*
	BitMem32_Copy((u32*)(Current_GICR_BASE + GICR_ICENABLER0), nPropertyWidth, intID, ~(pInt->isEnable));
	ctrl_Dist_WaitReadWriteFinished();

	// GICD 中也要设置
	u32 *addr = (u32*)(GICD_BASE + GICD_ISENABLE);
	u32 value = readl(addr);;
	value |= (1 << intID);
	writel(value, addr);
	*/
	//}

	//{ 不需要对 SGI/PPI 设置中断路由(其中PPI是当前PE私有的， 不可能路由给其它的PE， 然后SGI中断是指令触发的，并且在触发时可以设置接收端地址(dst PE Affinity))
	//}

	//{ 设置中断优先级，一个中断号占用一个字节
	nPropertyWidth = 8;
	BitMem32_Copy((u32*)(Current_GICR_BASE + GICR_IPRIORITYR), nPropertyWidth, intID, pInt->intr_pri);
	//}

	//{ 设置触发方式，一个中断号占用2个bits
	if(intID >= 16) // 无需对SGI中断设置触发方式
	{
		nPropertyWidth = 2;
		BitMem32_Copy((u32*)(Current_GICR_BASE + GICR_ICFGR0), nPropertyWidth, intID, pInt->intr_cfg);
	}
	//}
}


void GICv3_SPI_Config(GICv3Interrupt* pInt)
{
	s32 intID = pInt->intr_num;
	Need(intID >= 32 && intID < gicd.nSPI);

	u32 *addr;
	u32 value;
	s32 nPropertyWidth;

	//{ 设置中断分组，一个中断号占用1个bit
	u32 valueM;
	u32 valueG;
	switch(pInt->intr_grp){
		case IntGroupG0:
			valueM = 0; valueG = 0;
			break;
		case IntGroupG1S:
			valueM = 1; valueG = 0;
			break;
		case IntGroupG1NS:
		default:
			valueM = 0; valueG = 1;
			break;
	}
	nPropertyWidth = 1;
	BitMem32_Copy((u32*)(GICD_BASE + GICD_IGRPMODRn), nPropertyWidth, intID, valueM);
	BitMem32_Copy((u32*)(GICD_BASE + GICR_IGROUPR0),  nPropertyWidth, intID, valueG);
	//}

	//{ 设置中断开关，一个中断号占用1个bit
	nPropertyWidth = 1;
	BitMem32_Copy((u32*)(GICD_BASE + GICD_ISENABLE), nPropertyWidth, intID, pInt->isEnable);
	ctrl_Dist_WaitReadWriteFinished();
	//}
	
	//{ 设置中断路由，一个中断号占用64个bits
	u64 *addr64 = (u64*)(GICD_BASE + GICD_IROUTERn);
	u64 value64 = (u64)(pInt->dstAffinity);
	addr64 += intID;
	writel(value64, addr64);
	addr64 ++;
	//}
	
	//{ 设置中断优先级，一个中断号占用8bits
	nPropertyWidth = 8;
	BitMem32_Copy((u32*)(GICD_BASE + GICD_IPRIORITYR), nPropertyWidth, intID, pInt->intr_pri);
	//}

	//{ 设置触发方式，一个中断号占用2个bits
	nPropertyWidth = 2;
	BitMem32_Copy((u32*)(GICD_BASE + GICD_ICFGR), nPropertyWidth, intID, pInt->intr_cfg);
	//}
}


// 位内存操作(只能4字节读写的内存区暂称为 Mem32)
void BitMem32_Copy(u32 *base_addr/*基地址*/, s32 bitsPerUnit/*可为：1, 2, 4, 8, 16, 32*/, s32 iUnit/*索引*/, u32 newBits/*新值：在低位*/)
{
	u32 Mem32_mask;
	
	//Need(bitsPerUnit == 1 || bitsPerUnit== 2|| bitsPerUnit == 4||bitsPerUnit==8||bitsPerUnit==16||bitsPerUnit==32);
	Need((32 % bitsPerUnit) == 0);
	Need( ((s64)base_addr % 4) == 0);

	switch(bitsPerUnit)
	{
		case 1:  Mem32_mask = 0x00000001; break;
		case 2:  Mem32_mask = 0x00000003; break;
		case 4:  Mem32_mask = 0x0000000F; break;
		case 8:  Mem32_mask = 0x000000FF; break;
		case 16: Mem32_mask = 0x0000FFFF; break;
		case 32: Mem32_mask = 0xFFFFFFFF; break;
	}


	/* 每次可读写几个单位 */
	s32 unitPerOperation = 32 / bitsPerUnit;

	/* 计算索引和最终要访问的：地址、以及所需的移位量 */
	s32 Mem32_index = iUnit / unitPerOperation;
	s32 Mem32_shift = (iUnit % unitPerOperation) * bitsPerUnit;
	u32 *addr = base_addr + Mem32_index;

	// clear
	u32 value = readl(addr);
	value &= ~ (Mem32_mask << Mem32_shift);
	// set to new value
	value |= ((newBits & Mem32_mask) << Mem32_shift);
	writel(value, addr);
}
//}


//{ 中断默认配置
// 对 0..31 号中断进行分组
void GICv3_DefaultGrouping_SGI_PPI(s32 RD_Index, GICv3IntGroup defaultGrp)
{
	// return;

	/*
		中断分组，将0..31号共32个中断全部划到Group0(SGI、PPI set to G0S)
		划到Group0的中断，将全部由EL3层的代码响应、处理
		不会被路由到其它EL层

		注意：
		GICR_IGRPMODR 和 GICR_IGROUP 都是32位寄存器，
		仅可以对前32个中断号进行分组，
		前32个中断号是16个SGI和16个PPI中断
		
		规则(设中断号为 n)：
		n = 0，即中断号为0的中断
		GICR_IGRPMODR.bits[n] == 0 && GICR_IGROUP.bits[n] == 0，表示IRQ0划归G0S
		GICR_IGRPMODR.bits[n] == 1 && GICR_IGROUP.bits[n] == 0，表示IRQ0划归G1S
		GICR_IGRPMODR.bits[n] == 0 && GICR_IGROUP.bits[n] == 1，表示IRQ0划归G1NS

		n = 1，即中断号为1的中断
		GICR_IGRPMODR.bits[n] == 0 && GICR_IGROUP.bits[n] == 0，表示IRQ1划归G0S
		GICR_IGRPMODR.bits[n] == 1 && GICR_IGROUP.bits[n] == 0，表示IRQ1划归G1S
		GICR_IGRPMODR.bits[n] == 0 && GICR_IGROUP.bits[n] == 1，表示IRQ1划归G1NS
		。。。
		
	*/

	s32 maxIntID = 31;
	u32 *addrM, *addrG;
	u64 Current_GICR_BASE = get_RD_Base(RD_Index);
	s32 i;


	//{ 设置中断分组，一个中断号占用1个bit，因为每次都是对32位寄存器进行操作，每次可以处理32个中断号	
	u32 valueM = 0;
	u32 valueG = 0;
	switch(defaultGrp){
		case IntGroupG0:
			valueM = 0x0; valueG = 0x0;
			break;
		case IntGroupG1S:
			valueM = 0xFFFFFFFF; valueG = 0;
			break;
		case IntGroupG1NS:
		default:
			valueM = 0; valueG = 0xFFFFFFFF;
			break;
	}
	//通过 GICD_IGROUPRn 和 GICD_IGRPMODRn 寄存器进行配置
	addrM = (u32*)(u64)(Current_GICR_BASE + GICR_IGRPMODR0);
	addrG = (u32*)(u64)(Current_GICR_BASE + GICR_IGROUPR0);
	writel(valueM, addrM);
	writel(valueG, addrG);
	//}

	//{ 设置中断开关，一个中断号占用1个bit，默认禁用所有SPI/PPI中断
	u32 value = 0U;
	u32 *addr = (u32*)(u64)(Current_GICR_BASE + GICR_ISENABLER0);
	writel(value, addr);
	ctrl_RDist_WaitReadWriteFinished(RD_Index);
	//}

	//{ 不需要对 SGI/PPI 设置中断路由(其中PPI是当前PE私有的， 不可能路由给其它的PE， 然后SGI中断是指令触发的，并且在触发时可以设置接收端地址(dst PE Affinity))
	//}

	//{ 设置中断优先级，一个中断号占用一个字节，因为每次都是对32位寄存器进行操作，每次可以处理4个中断号
	addr = (u32*)(Current_GICR_BASE + GICR_IPRIORITYR);
	value = 0x80808080;
	for(i=0; i<= maxIntID; i+=4)
	{
		writel(value, addr);
		addr ++;
	}
	//}

	//{ 设置触发方式，一个中断号占用2个bits，因为每次都是对32位寄存器进行操作，所以每次可以处理16个中断号
	addr = (u32*)(Current_GICR_BASE + GICR_ICFGR0);
	value = 0x00000000;
	//writel(value, addr); // SGI 不需要设置触发方式！
	addr = (u32*)(Current_GICR_BASE + GICR_ICFGR0 + 4);
	writel(value, addr);
	//}
}

// 对 32..maxIntID 号中断进行分组
void GICv3_DefaultGrouping_SPI(GICD* pGicd, GICv3IntGroup defaultGrp)
{
	// return;

	/*
		//将中断号在 32..maxIntID 号范围内的SPI中断，全部划到Group0(SPI set to G0S)
		//划到Group0的中断，将全部由EL3层的代码响应、处理
		//不会被路由到其它EL层

		规则(设中断号为 n)：
		n = 0，即中断号为0的中断
		GICD_IGRPMODR.bits[n] == 0 && GICD_IGROUP[bits[n] == 0，表示IRQ0划归G0S
		GICD_IGRPMODR.bits[n] == 1 && GICD_IGROUP.bits[n] == 0，表示IRQ0划归G1S
		GICD_IGRPMODR.bits[n] == 0 && GICD_IGROUP.bits[n] == 1，表示IRQ0划归G1NS

		n = 1，即中断号为0的中断
		GICD_IGRPMODR.bits[n] == 0 && GICD_IGROUP.bits[n] == 0，表示IRQ1划归G0S
		GICD_IGRPMODR.bits[n] == 1 && GICD_IGROUP.bits[n] == 0，表示IRQ1划归G1S
		GICD_IGRPMODR.bits[n] == 0 && GICD_IGROUP.bits[n] == 1，表示IRQ1划归G1NS
		。。。

		*/
	
	u32 *addrM, *addrG;


	// SPI 中断号的分组， 也划分到G0S
	u32 valueM = 0;
	u32 valueG = 0;
	switch(defaultGrp){
		case IntGroupG0:
			valueM = 0; valueG = 0;
			break;
		case IntGroupG1S:
			valueM = 1; valueG = 0;
			break;
		case IntGroupG1NS:
		default:
			valueM = 0; valueG = 1;
			break;
	}

	s32 maxIntID = pGicd->nSPI;
	s32 i;

	//{ 设置中断分组，一个中断号占用1个bit，因为每次都是对32位寄存器进行操作，每次可以处理32个中断号
	// 跳过前面的32个中断号
	// 通过 GICD_IGROUPRn 和 GICD_IGRPMODRn 寄存器进行配置
	addrM = (u32*)(GICD_BASE + GICD_IGRPMODRn);
	addrG = (u32*)(GICD_BASE + GICD_IGROUPRn);
	addrM += 32 / 32;
	addrG += 32 / 32;
	for(i=MIN_SPI_ID; i< maxIntID; i+=32)
	{
		writel(valueM, addrM);
		writel(valueG, addrG);
		addrM ++;
		addrG ++;
	}
	//}

	//{ 设置中断开关，一个中断号占用1个bit，默认禁用所有SPI中断
	
	u32 *addr = (u32*)(GICD_BASE + GICD_ISENABLE);
	u32 value = 0x0;
	addr += 0;//32 / 32;
	for(i=/*MIN_SPI_ID*/0; i< maxIntID; i+=32)
	{
		writel(value, addr);
		ctrl_Dist_WaitReadWriteFinished();
		addr ++;
	}
	
	//}

	//{ 设置中断路由，一个中断号占用64个bits，默认全部跌幅到当前 Core
	u32 PEaffiniry = get_CurrPE_Affinity();
	u64 *addr64 = (u64*)(GICD_BASE + GICD_IROUTERn);
	u64 value64 = (u64)PEaffiniry;
	addr64 += 32;
	for(i=MIN_SPI_ID; i< maxIntID; i+=1)
	{
		writeq(value64, addr64);
		addr64 ++;
	}
	//}

	//{ 设置中断优先级，一个中断号占用一个字节，因为每次都是对32位寄存器进行操作，每次可以处理4个中断号
	addr = (u32*)(GICD_BASE + GICD_IPRIORITYR);
	value = 0x80808080;
	addr += 32 / 4;
	for(i=MIN_SPI_ID; i< maxIntID; i+=4)
	{
		writel(value, addr);
		addr ++;
	}
	//}
	
	//{ 设置触发方式，一个中断号占用2个bits，因为每次都是对32位寄存器进行操作，所以每次可以处理16个中断号
	addr = (u32*)(GICD_BASE + GICD_ICFGR);
	value = 0x00000000;
	addr += 32 / 16;
	for(i=MIN_SPI_ID; i< maxIntID; i+=16)
	{
		writel(value, addr);
		addr ++;
	}
	//}
}

//}



//{ GIC600 特有函数，不同于其它 GICv3 芯片的地方！！
	/* 主要是 gicv3_rdistif_off/on 的地方，参考：
	 * ===============================================================================
	 * E:\Dev\EE\Rockchip\arm-trusted-firmware-master\drivers\arm\gic\v3\gicv3_main.c
	 * 定义为 weak 
	 * #pragma weak gicv3_rdistif_off
	 * #pragma weak gicv3_rdistif_on

	 * E:\Dev\EE\Rockchip\arm-trusted-firmware-master\drivers\arm\gic\v3\gic-x00.c
	 * 在这里被重写
	 * void gicv3_rdistif_on(unsigned int proc_num)
	 * void gicv3_rdistif_off(unsigned int proc_num)
	*/


static void gicr_wait_group_not_in_transit(u64 Current_GICR_BASE)
{
#define PWRR_RDGPD_SHIFT		2
#define PWRR_RDGPO_SHIFT		3

#define PWRR_RDGPD				(1U << PWRR_RDGPD_SHIFT)
#define PWRR_RDGPO				(1U << PWRR_RDGPO_SHIFT)

	u32* addr = (u32*)(u64)(Current_GICR_BASE + GICR_PWRR);
	u32 pwrr;

	do {
		pwrr = readl(addr);

	/* Check group not transitioning: RDGPD == RDGPO */
	} while (((pwrr & PWRR_RDGPD) >> PWRR_RDGPD_SHIFT) !=
		 ((pwrr & PWRR_RDGPO) >> PWRR_RDGPO_SHIFT));
}

void gicv3_rdistif_on(s32 RD_Index)
{
// #ifdef PLAT_QEMU
	// return;
// #endif

#define PWRR_RDPD_SHIFT			0

#define PWRR_ON				(0U << PWRR_RDPD_SHIFT)
#define PWRR_RDPD			(1U << PWRR_RDPD_SHIFT)

	if(!get_RD_need_power_mgmt(RD_Index)) return;

	printf("pwr_on GIC600 ReDistributor\r\n");
	
	//gic600_pwr_on
	u64 Current_GICR_BASE = get_RD_Base(RD_Index);
	u32* addr = (u32*)(u64)(Current_GICR_BASE + GICR_PWRR);
	u32 value = readl(addr);
	printf("\tGICR_PWRR: 0x%08x\r\n", value);


	do {	/* Wait until group not transitioning */
		gicr_wait_group_not_in_transit(Current_GICR_BASE);

		/* Power on redistributor */
		writel(PWRR_ON, addr);

		/*
		 * Wait until the power on state is reflected.
		 * If RDPD == 0 then powered on.
		 */
		value = readl(addr);
	} while ((value & PWRR_RDPD) != PWRR_ON);
	printf("\tGICR_PWRR: 0x%08x\r\n", value);
}

void gicv3_rdistif_off(unsigned int proc_num)
{
	// todo
	// 未完成
}

//}


//{ 辅助函数
s32 get_CurrentEL()
{
	u32 value = read_CurrentEL();
	value = ( value >> 2 ) & 0x3;
	return (s32)(value);
}

s32 is_in_el3()
{
	return (get_CurrentEL() == 3)?1:0;
}

u32 get_gic_version(GICD* pGicd)
{
	u64 addr_ = (u64)(pGicd->gicdBase + GICD_PIDR2_GICV3);
	u32 value = readl((u32*)addr_);
	return (value>>4) & 0xF;
}

s32 get_Max_SPI_IntID(GICD* pGicd)
{
	// 参考：arm-trusted-firmware-master   \drivers\arm\gic\v3\gicv3_helpers.c
	
	const s32 MAX_SPI_ID = 1019; 

	// 读取 GICD_TYPER.ITLinesNumber【低5bits】  中断信号线条数 (ITLines) ？？
	u32 addr_ = (u32)(pGicd->gicdBase + GICD_TYPER);
	u32 value = readl((u32*)(u64)addr_);
	value = (( (value & 0x1f) + 1 ) * 32);

	// special INTIDs 1020-1023 有特殊用途， 不准超了
	if(value > MAX_SPI_ID) value = 1019;
	return (int)value;
}

u32 get_CurrPE_Affinity()
{
	/*
	mrs	x1, mpidr_el1		//执行完 aff0..aff1 在x1 bit:0..bit:23处，aff3在 x1 bit:32~bit39
	lsr	x0, x1, #32			//x1右移，执行完 aff3在 x0 bit:0~bit7
	bfi	x1, x0, #24, #8	    //x1低8位位段插入到 x1[24]处，执行完  x1 低32位即是 aff3:aff2:aff1:aff0
	mov x0, xzr
	mov w0, w1
	*/
	u64 value = read_MPIDR_EL1();
	u32 aff3 = (u32)( (value >> 32) & 0xF);
	u32 aff_ = (u32)( value & 0x00FFffff);
	
	return aff3 | aff_;
}

s32 get_RD_Count()
{	
	s32 i;
	for(i=0; i<gicd.MAX_RD_COUNT;i++)
	{
		u64 typeVal = get_RD_TypeValue(i);
		if(get_RD_IsLastOne_FromTypeValue(typeVal))
		{
			return i+1;
		}
	}
	
	return 0;
}

s32 get_CurrRD()
{
	s32 n = get_RD_Count();
	for(s32 i=0; i< n;i++)
	{
		u64 typeVal = get_RD_TypeValue(i);
		u32 RDaffiniry = get_RD_Affinity_FromTypeValue(typeVal);
		u32 PEaffiniry = get_CurrPE_Affinity();
		
		if(RDaffiniry == PEaffiniry) 
		{
			return i;
		}
	}
	
	return -1;
}

u64 get_RD_Base(s32 index)
{
	// GICv3 每个 ReDistributor 固定占用 1 << 17 字节的空间(128kb)，可参考ARM官方 ATF 源码
	Need(GICR_BASE != 0U);
	Need(index >= 0);
	return GICR_BASE + ( index * (2 << 16));
}

u32 get_RD_modelID(s32 RD_Index)
{
#define IIDR_PRODUCT_ID_MASK	U(0xff000000)
#define IIDR_IMPLEMENTER_MASK	U(0x00000fff)
#define IIDR_MODEL_MASK			(IIDR_PRODUCT_ID_MASK | IIDR_IMPLEMENTER_MASK)

	u64 Current_GICR_BASE = get_RD_Base(RD_Index);
	u32* addr = (u32*)(u64)(Current_GICR_BASE + GICR_IIDR);
	u32 value = readl(addr);
	value &= IIDR_MODEL_MASK;
	return value;
}

u32 get_RD_need_power_mgmt(s32 RD_Index)
{
	switch( gicd.gic_modelID )
	{
		case IIDR_MODEL_ARM_GIC_600:
		case IIDR_MODEL_ARM_GIC_600AE:
		case IIDR_MODEL_ARM_GIC_700:
			return 1;
		default:
			return 0;
	}
}

u64 get_RD_TypeValue(s32 index)
{
	Need(index >= 0);
	u64 RDBase = get_RD_Base(index);
	
	u64* addr = (u64*)(RDBase + GICR_TYPER);
	u64 value = readq((u64*)addr);
	return value;
}

u32 get_Dist_CtlrValue()
{
	u32* addr = (u32*)(u64)(gicd.gicdBase + GICD_CTLR);
	u32 value = readl(addr);
	return value;
}

u32 get_RDist_CtlrValue(s32 RD_Index)
{
	u64 Current_GICR_BASE = get_RD_Base(RD_Index);
	u32* addr = (u32*)(u64)(Current_GICR_BASE + GICR_CTLR);
	u32 value = readl(addr);
	return value;
}

u32 get_RD_Affinity_FromTypeValue(u64 TypeValue)
{
	// 读取 GICR_TYPER.AffAddr【高位32bits】
	u32 value = (TypeValue >> 32) & 0xFFFFFFFF;
	return (u32)value;
}

u32 get_RD_ProcessorID_FromTypeValue(u64 TypeValue)
{
	// 读取 GICR_TYPER.ProcessorNumber【bit8开始的16bits】
	u32 value = (TypeValue >> 8) & 0x0000FFFF;
	return (u32)value;
}

s32 get_RD_IsLastOne_FromTypeValue(u64 TypeValue)
{
	return (TypeValue & (1<<4)) != 0;
}

void ctrl_RD_wakeup_TheConnectedCpuif(s32 RD_Index)
{	
	/*
	BIT0: 厂商自定义值
	BIT1: ProcessorSleep 代码中只能改这个位，而不能改下面的 CA 位(应该是改也无效)
	BIT2: ChildrenASleep(1<<2)，此bit为1，表示cpuif处于睡眠状态
	*/

	u64 Current_GICR_BASE = get_RD_Base(RD_Index);
	u32* addr = (u32*)(u64)(Current_GICR_BASE + GICR_WAKER);
	u32 value = readl(addr);

	printf("\tGICR_WAKER: 0x%08x\r\n", value);

	u32 is_cpuif_offline = (value & (1<<2)) != 0; 	// 通过 CA 位判断PE现在是睡着的吗
	printf("\tis_cpuif_offline: %d\r\n", is_cpuif_offline);
	if(is_cpuif_offline)
	{
		value = ~0x2;;//~(1<<1);
		writel(value, addr);						// Clear ProcessorSleep bit to 叫醒 cpuif 
		dsb();
		isb();

		// 等待唤醒成功后再继续
		do{
			value = readl(addr);
			is_cpuif_offline = (value & (1<<2)) != 0; // 通过 CA 位判断PE现在是睡着的吗
		}while(is_cpuif_offline);
	}
	printf("\tGICR_WAKER: 0x%08x\r\n", value);
}

void ctrl_RD__sleep_TheConnectedCpuif(s32 RD_Index)
{
	/*
	ProcessorSleep BIT: (1<<1)，代码中只能改这个位，而不能改下面的 CA 位(应该是改也无效)
	ChildrenASleep BIT: (1<<2)，此bit为1，表示ThePE处于睡眠状态
	*/

	u64 Current_GICR_BASE = get_RD_Base(RD_Index);
	u32* addr = (u32*)(u64)(Current_GICR_BASE + GICR_WAKER);
	u32 value = readl(addr);
	value |= (1<<1);
	writel(value, addr);		// set ProcessorSleep
	dsb();
	isb();

	// 等待睡眠成功后再继续
	do{
		value = readl(addr);
	}while((value & (1<<2)) == 0); // ChildrenASleep BIT == 0，表示没睡着
}

/* Wait for updates to:
 * GICD_CTLR[2:0] - the Group Enables
 * GICD_CTLR[7:4] - the ARE bits, E1NWF bit and DS bit
 * GICD_ICENABLER<n> - the clearing of enable state for SPIs
 */
void ctrl_Dist_WaitReadWriteFinished()
{
	while ((get_Dist_CtlrValue() & (1<<31)) != 0U) {
	}
}

void ctrl_RDist_WaitReadWriteFinished(s32 RD_Index)
{
	while ((get_RDist_CtlrValue(RD_Index) & (1<<3)) != 0U) {
	}
}

void gicv3_raise_G0S_SGI(s32 IntID, u32 TargetList)
{
	/* 参考：
	GICv3_Software_Overview_Official_Release_B.pdf
	7.1 Generating SGIs
	*/

	Need(IntID >= 0 && IntID < 16);

	u32 rangeSelector = 0;

	u64 value = 0;
	value |= ((u64)(rangeSelector & 0xF) << 44);	// RS 共4bits，表示第几组 TargetList
	value &= ~((u64)1 << 40);						// CLR IRM to use affinity routing
	value |= ((IntID & 0xF) << 24);				// 中断号, 共4bits
	value |= ((TargetList & 0xFFFF) << 0);			// TargetList共16bits，每bit代表一个目标PE
													// 比如bit1代表要发给第一颗PE, bit2代表要发给第二颗PE

	asm volatile ("dsb ishst" : : : "memory");
	write_ICC_SGI0R_EL1(value);						// ICC_SGI0R_EL1，代表发送的是 TEE Group0 的 SGI

	/* 关于 ICC_SGI0R_EL1
	软件写ICC_SGI0R_EL1产生secure状态的group0软中断
	软件写ICC_ASGI1R_EL1产生secure状态的group1软中断
	软件写ICC_SGI1R_EL1产生对应当前secure状态的group1软中断
	*/
	
	isb();

}

void RaiseTestInt(s32 IntID)
{

	Need(IntID>=0 && IntID < gicd.nSPI);
	
	u32* addr;
	u32 value;
	
	if(IntID<32)
	{
		u64 Current_GICR_BASE = get_RD_Base(0);
		addr = (u32*)(Current_GICR_BASE + GICR_ISACTIVER0);
		value = 0x1 << IntID;
		
		writel(value, addr);
	}
	else
	{
		addr = (u32*)(GICD_BASE + GICD_ISACTIVEREn);
		addr += IntID / 32;
		s32 shift = IntID % 32;
		value = 0x1 << shift;

		writel(value, addr);
	}
	
	printf("RaiseTestInt: %016x\r\n", *addr);
}


//}


//{ 中断处理函数

void SYNHandler(void)
{
	printf("[ERR] %s\r\n", __func__);
}

void fiqHandler(void)
{
	u64 value = read_ICC_IAR0_EL1();
	
	printf("[ERR] %s\r\n", __func__);
	
	vApplicationFIQHandler(value & 0x3FF);
	
	write_ICC_EOIR0_EL1(value);
	
	/*
    int irq;

    irq = readl(GICC_BASE + GICC_IAR);

    if((irq & 0x3ff) == 29) {
		ISR_TeeTimer();
    }
    writel(irq, GICC_BASE + GICC_EOIR);
	*/
}

void irqHandler(void)
{
	printf("[ERR] %s\r\n", __func__);
	/*
    int irq;

    irq = readl(GICC_BASE + GICC_IAR);

    if((irq & 0x3ff) == 29) {
		ISR_TeeTimer();
    }
    writel(irq, GICC_BASE + GICC_EOIR);
	*/
}

void ErrHandler(void)
{
	printf("[ERR] %s\r\n", __func__);
}
//}

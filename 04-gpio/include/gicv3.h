#ifndef __GIC_H
#define __GIC_H

#include "common.h"


// rk3568 gicv3.h	2022.02.14	Y整理				(参考源码：https://github.com/ARM-software/arm-trusted-firmware/tree/master/drivers/arm/gic/v3)

 /*	GICv3 介绍

	********************************************************************************
	PE: Process Element, 就是cpu core，即CPU核心，一颗CPU内可以有几个core(PE)

	中断体系配置
		既有全局性的配置
		也有各PE自己独立的配置

	GIC 和 CPU 的硬件连接结构
		GIC 内有：一个 Distributor  +  N 个 ReDistributor

		CPU 内有：N 组 【Core + CPUInterface】，(其中一组是主组，做为启动组，启动组的核心称为主核，即 master PE )
		CPU 内每组 【CPUInterface + core】 都连接到 GIC 内的一个 ReDistributor。
		CPU 侧的 CPUInterface 到 GIC 侧的 ReDistributor 之间，有 IRQ、FIQ 信号线。

			Distributor   层有 GICD_CTLR 来开关 G0S, G1S, G1NS 三组的中断
			ReDistributor 层有 GICR_CTLR 来开关 G0S, G1S, G1NS 三组的中断
			CPUInterface  无需配置
			PE            层有 DAIFClr/DAIFSet 来开关 Debug、SError、IRQ、FIQ类中断

	寄存器Banked
		32个通用寄存器x0..x31 和 运算逻辑相关的寄存器，每个PE都有一组此类寄存器(即Banked)
		部分特殊寄存器 和 映射到内存地址的寄存器，则是所有PE共享的。
		
		比如每个PE的地址，都保存在自己的 MPIDR_EL1 寄存器中(官方文档称为 affinity 地址，亲和性地址)
		aff地址格式：aff3.aff2.aff1.aff0 
			(很像IP地址，符号"."隔开的每个段, 占8个bits，总共32bits即可表示一个完整affinity地址)

	本代码(rk3568spl_2GIC)配置的环境：
		只使用master核心，所有的slavePE通过入口处 SleepIfSlaveCore 的代码，明确让其空转
		无REE(即无no-secure环境)，只有 (EL3 SecureMonitor) + (EL1/EL0 TEE) 
		只使用EL3 SM, 不写 TEE EL1/EL0 的代码
		所有代码都运行于主 PE 的 EL3(SM)

	本代码(rk3568spl_2GIC)功能：
		基于 rk3568spl_1Uart 修改，增加GIC控制代码，增加 EL3 TEE Timer 定时器代码。

	********************************************************************************
*/

//{GIC芯片型号
#define IIDR_MODEL_ARM_GIC_600		U(0x0200043b)
#define IIDR_MODEL_ARM_GIC_600AE	U(0x0300043b)
#define IIDR_MODEL_ARM_GIC_700		U(0x0400043b)
//}

//{ GICv3 基地址(rk3568)
/* GIC 中的 Distributor 基地址						(在 《Rockchip RK3568 TRM Part1 V1.1-20210301.pdf》 中 1.1 Address Mapping 小节 GIC600 处)
 * ===================================================================
 */
#ifdef PLAT_QEMU
#define GICD_BASE			0x8000000
#else
#define GICD_BASE			0xfd400000
#endif
/* RD 介绍 及 GIC 中的 ReDistributor(RD) 基地址		(参考源码： u-boot-next-dev   \arch\arm\include\asm\gic.h)
 * 因为gic内有多个 RDdistributor(RD)
 * 每个 RD 占用内存地址空间为：0x20000(2<<16，1<<17) = 128kb
 * 每个 RD 的128kb空间又分为:
 *    64kb 的 SPI & RD  专属配置空间(64kb = 0x10000，1<<16)
 *    64kb 的 SGI & PPI 专属配置空间
 */
#ifdef PLAT_QEMU
#define GICR_BASE			0x80a0000
#else
#define GICR_BASE			0xfd460000
#endif

/* GIC 中的 CPUInterface 基地址
   GICv3不用 GICC_* 这种 MMIO 的方式配置 cpuif!!!
   而是使用 ICC_* 这种类似协寄存器的方式配置 cpuif !!
   如果 SoC 中集成的 GICv3 芯片兼容 GICv2，则：
		1). GICD_CTLR.ARE_S 位，在 reset 时为0(ATF 源码中通过此位来判断是否兼容 GICv2 )
		2). ICC_SRE_EL3.SRE 位，在 reset 时为1(RAO/WI, 即read as one & write ignore)
		3). 如果硬是有芯片把 ICC_SRE_EL3.SRE 默认设置为 0，那就一定要给 GICC_BASE ，
			不然无法配置 cpuif（因为当 ICC_SRE_EL3.SRE 为0时，本身就无法读取 ICC_SRE_EL3 寄存器，会触发中断，而此时中断系统还没配置完成）
   本代码没兼容 GICv2 ！！
*/

//}


//{ GICv3 规定的各种寄存器的具体偏移值				(参考源码：https://github.com/ARM-software/arm-trusted-firmware/tree/master/drivers/arm/gic/v3) */


	//{GICD_* 寄存器偏移定义
	#define GICD_CTLR		0x0000
	#define GICD_TYPER		0x0004

	/* GICD_IGROUPRn 和 GICD_IGRPMODRn 介绍
	 * GICD_IGROUPRn 处开始的一小片内存，都用于对 SPI 中断号( 32..maxIntId )进行分组
	 * 注意：
	 *   最前面的32bits(4bytes)不用配置
	 * 对于支持 320 个SPI中断的板子，GICD_IGROUPRn 需要 320/8 = 40 个字节就够用 ？
	 */
	#define GICD_IGROUPRn	0x0080
	#define GICD_IGRPMODRn	0x0D00
	
	/* 是否允许该中断 */
	#define GICD_ISENABLE	0x100
	#define GICD_ICENABLE	0x180

	/* 优先级，每个中断号占用1个字节 */
	#define GICD_IPRIORITYR	0x0400
	
	/* 中断触发方式，每个中断号占用2个bits */
	#define GICD_ICFGR		0x0C00

	// 测试用的，模拟触发中断用的寄存器！
	#define GICD_ISACTIVEREn	0x1a00
	#define GICD_ICACTIVEREn	0x1c00

	/* GICD_IROUTERn 介绍
	 * 参考：
	 * 1). https://github.com/ARM-software/arm-trusted-firmware/blob/61e30277199e5457483bef791cb5bc026c402a1f/include/drivers/arm/gicv3.h
	 * 2). u-boot-next-dev  \arch\arm\include\asm\gic.h
	 *
	 * GICD_IROUTER<n> register is at 0x6000 + 8n, where n is the interrupt id and
	 * n >= 32, making the effective offset as 0x6100.
	 *
	 * 程序员视角就是
	 * (u64*)GICD_IROUTERn = 0x6000;
	 * GICD_IROUTERn[0..31]			for SGI & PPI， 每个中断号的路由配置数据占用8个字节，总占用8*32=256=0x100个字节
	 * GICD_IROUTERn[31...maxIntId]	for SPI
	 */
	#define GICD_IROUTERn		0x6000
	#define GICD_PIDR0_GICV3	0xffe0
	#define GICD_PIDR1_GICV3	0xffe4
	#define GICD_PIDR2_GICV3	0xffe8
	//}



	//{GICR_* 寄存器偏移定义
	#define GICR_CTLR		0x0000			// width32
	#define GICR_IIDR		0x04
	#define GICR_TYPER		0x0008			// Width64, GIC600使用这里面的地址来分发LPI中断
	#define GICR_WAKER		0x0014
	#define GICR_PWRR		0x24U


	#define GICR_SGIBASE_OFFSET	(1 << 0x10)	// 64 KB
	#define GICR_IGROUPR0		(GICR_SGIBASE_OFFSET + 0x80)
	#define GICR_ISENABLER0		(GICR_SGIBASE_OFFSET + 0x100)
	#define GICR_ICENABLER0		(GICR_SGIBASE_OFFSET + 0x180)
	#define GICR_ISACTIVER0		(GICR_SGIBASE_OFFSET + 0x300)
	#define GICR_ICACTIVER0		(GICR_SGIBASE_OFFSET + 0x380)
	#define GICR_IPRIORITYR		(GICR_SGIBASE_OFFSET + 0x400)
	#define GICR_ICFGR0			(GICR_SGIBASE_OFFSET + 0xc00)
	#define GICR_ICFGR1			(GICR_SGIBASE_OFFSET + 0xc04)
	#define GICR_IGRPMODR0		(GICR_SGIBASE_OFFSET + 0xd00)
	//}




	//{GICC_* 寄存器偏移定义(GICv3不用 GICC_* 这种MMIO的方式配置 cpuif!)
 
	/* GICv3不用 GICC_* 这种 MMIO 的方式配置 cpuif!!!
	而是使用 ICC_* 这种类似协寄存器的方式配置 cpuif !!
	如果 SoC 中集成的 GICv3 芯片兼容 GICv2，则：
		1). GICD_CTLR.ARE_S 位，在 reset 时为0(ATF 源码中通过此位来判断是否兼容 GICv2 )
		2). ICC_SRE_EL3.SRE 位，在 reset 时为1(RAO/WI, 即read as one & write ignore)
		3). 如果硬是有芯片把 ICC_SRE_EL3.SRE 默认设置为 0，那就一定要给 GICC_BASE ，
			不然无法配置 cpuif（因为当 ICC_SRE_EL3.SRE 为0时，本身就无法读取 ICC_SRE_EL3 寄存器，会触发中断，而此时中断系统还没配置完成）
	本代码没兼容 GICv2 ！！
	*/

	#define _ICC_IAR0_EL1		S3_0_C12_C8_0
	#define _ICC_IAR1_EL1		S3_0_C12_C12_0
	#define _ICC_EOIR0_EL1		S3_0_C12_C8_1
	#define _ICC_EOIR1_EL1		S3_0_C12_C12_1
	#define _ICC_HPPIR0_EL1		S3_0_C12_C8_2
	#define _ICC_HPPIR1_EL1		S3_0_C12_C12_2
	#define _ICC_BPR0_EL1		S3_0_C12_C8_3
	#define _ICC_BPR1_EL1		S3_0_C12_C12_3
	#define _ICC_DIR_EL1		S3_0_C12_C11_1
	#define _ICC_PMR_EL1		S3_0_C4_C6_0
	#define _ICC_RPR_EL1		S3_0_C12_C11_3
	#define _ICC_CTLR_EL1		S3_0_C12_C12_4
	#define _ICC_CTLR_EL3		S3_6_C12_C12_4	// ICC_CTLR_EL3 等效于 GICC_CTLR
	#define _ICC_SRE_EL1		S3_0_C12_C12_5
	#define _ICC_SRE_EL2		S3_4_C12_C9_5
	#define _ICC_SRE_EL3		S3_6_C12_C12_5
	#define _ICC_IGRPEN0_EL1	S3_0_C12_C12_6
	#define _ICC_IGRPEN1_EL1	S3_0_C12_C12_7
	#define _ICC_IGRPEN1_EL3	S3_6_C12_C12_7
	#define _ICC_SEIEN_EL1		S3_0_C12_C13_0
	#define _ICC_SGI0R_EL1		S3_0_C12_C11_7
	#define _ICC_SGI1R_EL1		S3_0_C12_C11_5
	#define _ICC_ASGI1R_EL1		S3_0_C12_C11_6
	//}
//}




//{ GICv3 代码

#define MIN_SPI_ID 32

#define GICD_READL(gicd_reg_offset)  (readl((GICD_BASE)			+ (gicd_reg_offset)))
#define GICR_READL(gicr_reg_offset)  (readl((Current_GICR_BASE)	+ (gicr_reg_offset)))
#define GICD_READQ(gicd_reg_offset)  (readq((GICD_BASE)			+ (gicd_reg_offset)))
#define GICR_READQ(gicr_reg_offset)  (readq((Current_GICR_BASE)	+ (gicr_reg_offset)))

#define GICD_WRITEL(v, gicd_reg_offset)  (writel((v), (GICD_BASE)			+ (gicd_reg_offset)))
#define GICR_WRITEL(v, gicr_reg_offset)  (writel((v), (Current_GICR_BASE)	+ (gicr_reg_offset)))
#define GICD_WRITEQ(v, gicd_reg_offset)  (writeq((v), (GICD_BASE)			+ (gicd_reg_offset)))
#define GICR_WRITEQ(v, gicr_reg_offset)  (writeq((v), (Current_GICR_BASE)	+ (gicr_reg_offset)))


typedef enum{
	IntGroupG0  = 0,// GRPMODR == 0 && GROUP == 0
	IntGroupG1S = 1,// GRPMODR == 1 && GROUP == 0
	IntGroupG1NS= 2,// GRPMODR == 0 && GROUP == 1
}GICv3IntGroup;

typedef void (*GICv3InterruptHandler)();

typedef struct {
	s32				intr_num:10;	//中断号
	GICv3IntGroup	intr_grp:2;		//组别  ，  每个中断占用2bites空间(包含了安全状态)，但分开到两个不同的寄存器，每个寄存器中各存一位
	u32				intr_pri:8;		//优先级，  每个中断占用8bites空间(实际只使用低5bits，即支持32个中断优先级，见：corelink_gic600_generic_interrupt_controller_technical_reference_manual_100336_0106_00_en.pdf 第203页 "Implementation-defined features reference")
	u32				intr_cfg:2;		//触发方式，每个中断占用2bites空间
	u32				dstAffinity;	//路由到哪个PE
	u32				isEnable:1;		//是否启用
	GICv3InterruptHandler hanlder;	//该中断触发时的处理函数
} GICv3Interrupt;


typedef struct {
	u64 gicdBase;
	u64 gicrBase;
	
	u32 gic_modelID;
	u32 gic_version;
	s32 nReDistributor;
	s32	MAX_RD_COUNT;
	s32 nProcessor;
	s32 nSPI;
	u32 CurrPE_Affinity;
	s32 CurrRD;
	u32 GICv3_SRE;
} GICD;



extern void RaiseTestInt(s32 IntID);
extern void GICv3_Demo();
extern s32  GICv3Init();
extern void GICv3_Config_Distributor();
extern void GICv3_Config_ReDistributor(s32 RD_Index);
extern void GICv3_Config_CPUInterface(s32 RD_Index);
extern void GICv3_Config_PE();

extern s32 get_CurrentEL();
extern u32 get_CurrPE_Affinity();
extern u32 get_gic_version(GICD* pGicd);
extern s32 get_Max_SPI_IntID(GICD* pGicd);

extern u64 get_RD_Base(s32 index);
extern u32 get_RD_modelID(s32 RD_Index);
extern u64 get_RD_TypeValue(s32 index);
extern u32 get_Dist_CtlrValue();
extern u32 get_RDist_CtlrValue(s32 RD_Index);
extern s32 get_RD_Count();
extern s32 get_CurrRD();
extern u32 get_RD_Affinity_FromTypeValue(u64 TypeValue);
extern u32 get_RD_ProcessorID_FromTypeValue(u64 TypeValue);
extern s32 get_RD_IsLastOne_FromTypeValue(u64 TypeValue);

extern u32 get_RD_need_power_mgmt(s32 RD_Index);
extern void gicv3_rdistif_on(s32 RD_Index);
extern void ctrl_RD_wakeup_TheConnectedCpuif(s32 RD_Index);
extern void ctrl_RD__sleep_TheConnectedCpuif(s32 RD_Index);
extern void ctrl_Dist_WaitReadWriteFinished();
extern void ctrl_RDist_WaitReadWriteFinished(s32 RD_Index);

extern void TEE_REE_Setup();
extern void GICv3_InterruptConfig(GICv3Interrupt* pInt);
extern void GICv3_SGI_PPI_Config(GICv3Interrupt* pInt, s32 RD_Index);
extern void GICv3_SPI_Config(GICv3Interrupt* pInt);
extern void BitMem32_Copy(u32 *base_addr/*基地址*/, s32 bitsPerUnit/*可为：1, 2, 4, 8, 16, 32*/, s32 iUnit/*索引*/, u32 newBits/*新值：在低位*/);


extern void PECfg_ClsIntMask();


extern void GICv3_DefaultGrouping_SGI_PPI(s32 RD_Index, GICv3IntGroup defaultGrp);
extern void GICv3_DefaultGrouping_SPI(GICD* pGicd, GICv3IntGroup defaultGrp);

extern void gicv3_raise_G0S_SGI(s32 IntID, u32 TargetList);
//}


//{ C语言读写 特殊寄存、通用寄存器、协寄存器 定义

// 特殊 msr mrs
DEFINE_SysREG_RW_FUNCS(u32, CurrentEL)
DEFINE_SysREG_RW_FUNCS(u32, sctlr_el3)
DEFINE_SysREG_RW_FUNCS(u32, daif)
DEFINE_SysREG_RW_FUNCS(u64, vbar_el3)
DEFINE_SysREG_RW_FUNCS(u32, cptr_el3)
DEFINE_SysREG_RW_FUNCS(u64, cntfrq_el0)
DEFINE_SysREG_RW_FUNCS(u32, midr_el1)
DEFINE_SysREG_RW_FUNCS(u64, MPIDR_EL1)
DEFINE_SysREG_RW_FUNCS(u32, spsr_el3)
DEFINE_SysREG_RW_FUNCS(u32, ID_AA64PFR0_EL1)
DEFINE_SysREG_RW_FUNCS(u32, SPSel)
DEFINE_SysREG_RW_FUNCS(u64, scr_el3)

// 通用 mov
DEFINE_GenREG_RW_FUNCS(u64, SP)
DEFINE_GenREG_RW_FUNCS(u64, x30)

// cpuif 寄存器 mrs msr
/*
ICC_SRE_EL3 官方说明：
	https://developer.arm.com/documentation/ddi0601/2021-12/AArch64-Registers/ICC-SRE-EL3--Interrupt-Controller-System-Register-Enable-register--EL3-?lang=en

atf 中对 ICC_SRE_EL3 寄存的定义:
#define ICC_MSRE	p15, 6, c12, c12, 5
	__asm__ volatile ("mrc "#coproc","#opc1",%0,"#CRn","#CRm","#opc2 : "=r" (v));\
	__asm__ volatile ("mcr "#coproc","#opc1",%0,"#CRn","#CRm","#opc2 : : "r" (v));\
	
	参考：Arm® Generic Interrupt Controller Architecture Specification, GIC architecture version 3.0 and version 4.0.pdf 中
		"Accessing the ICC_SRE_EL3:" 处！
		"Register access is encoded as follows:" 然后右边有几个像协处理器寄存器的格式！

*/
DEFINE_CPUIFR_RW_FUNCS(u32, ICC_SRE_EL3, 		_ICC_SRE_EL3)
DEFINE_CPUIFR_RW_FUNCS(u32, ICC_IGRPEN0_EL1,	_ICC_IGRPEN0_EL1)	// 用于配置EL1/2/3层是否允许group0中断(注：group0 只给TEE侧使用)
DEFINE_CPUIFR_RW_FUNCS(u32, ICC_IGRPEN1_EL1,	_ICC_IGRPEN1_EL1)	// 用于配置EL1层是否允许group1中断
DEFINE_CPUIFR_RW_FUNCS(u32, ICC_IGRPEN1_EL3,	_ICC_IGRPEN1_EL3)	// 用于配置EL3层是否允许group1中断
DEFINE_CPUIFR_RW_FUNCS(u32, ICC_PMR_EL1,		_ICC_PMR_EL1)
DEFINE_CPUIFR_RW_FUNCS(u64, ICC_SGI0R_EL1,		_ICC_SGI0R_EL1)
DEFINE_CPUIFR_RW_FUNCS(u32, ICC_CTLR_EL3,		_ICC_CTLR_EL3)

DEFINE_CPUIFR_RW_FUNCS(u64, ICC_BPR0_EL1, 		_ICC_BPR0_EL1)
DEFINE_CPUIFR_RW_FUNCS(u32, ICC_IAR0_EL1,		_ICC_IAR0_EL1)
DEFINE_CPUIFR_RW_FUNCS(u32, ICC_EOIR0_EL1,		_ICC_EOIR0_EL1)


/*
ICC_IAR0_EL1 Used to acknowledge Group 0 interrupts.
ICC_IAR1_EL1 Used to acknowledge Group 1 interrupts.
*/



// 协寄存器 mrc mcr

//}


#endif


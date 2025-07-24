#ifndef _COMMON_H
#define _COMMON_H

/**********************************************************/
/************************** 编译开关 ************************/
/**********************************************************/
#define Use_InfoDump	1
#define Use_ASSERTIONS	1


#include "printf.h"
//{common type 定义
/**********************************************************/
/************************** common ************************/
/**********************************************************/
typedef unsigned long   size_t;

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

# define  U_(_x)	(_x##U)
# define   U(_x)	U_(_x)
# define  UL(_x)	(_x##UL)
# define ULL(_x)	(_x##ULL)
# define   L(_x)	(_x##L)
# define  LL(_x)	(_x##LL)


#define ISB	asm volatile ("isb sy" : : : "memory")
#define DSB	asm volatile ("dsb sy" : : : "memory")
#define DMB	asm volatile ("dmb sy" : : : "memory")

#define isb()	ISB
#define dsb()	DSB
#define dmb()	DMB

#define __arch_getb(a)			(*(volatile unsigned char *)(a))
#define __arch_getw(a)			(*(volatile unsigned short *)(a))
#define __arch_getl(a)			(*(volatile unsigned int *)(a))
#define __arch_getq(a)			(*(volatile unsigned long long *)(a))

#define __arch_putb(v,a)		(*(volatile unsigned char *)(a) = (v))
#define __arch_putw(v,a)		(*(volatile unsigned short *)(a) = (v))
#define __arch_putl(v,a)		(*(volatile unsigned int *)(a) = (v))
#define __arch_putq(v,a)		(*(volatile unsigned long long *)(a) = (v))

#define mb()		dsb()
#define __iormb()	dmb()
#define __iowmb()	dmb()

#define readb(c)	({ u8  __v = __arch_getb(c); __iormb(); __v; })
#define readw(c)	({ u16 __v = __arch_getw(c); __iormb(); __v; })
#define readl(c)	({ u32 __v = __arch_getl(c); __iormb(); __v; })
#define readq(c)	({ u64 __v = __arch_getq(c); __iormb(); __v; })

#define writeb(v,c)	({ u8  __v = v; __iowmb(); __arch_putb(__v,c); __v; })
#define writew(v,c)	({ u16 __v = v; __iowmb(); __arch_putw(__v,c); __v; })
#define writel(v,c)	({ u32 __v = v; __iowmb(); __arch_putl(__v,c); __v; })
#define writeq(v,c)	({ u64 __v = v; __iowmb(); __arch_putq(__v,c); __v; })

#define rk_clrsetreg(addr, clr, set)	writel(((clr) | (set)) << 16 | (set), addr)
#define rk_clrreg(addr, clr)			writel((clr) << 16, addr)
#define rk_setreg(addr, set)			writel((set) << 16 | (set), addr)

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define check_member(structure, member, offset) _Static_assert( \
	offsetof(struct structure, member) == offset, 				\
	"`struct " #structure "` offset for `" #member "` is not " #offset)
//}


//{ C语言访问寄存器相关宏
// mrs 寄存器访问相关定义
#define DEFINE_SysREG_RW_FUNCS(_type, _name)				\
	_DEFINE_SysREG_READ__FUNC (_type, _name, _name)			\
	_DEFINE_SysREG_WRITE_FUNC (_type, _name, _name)

#define _DEFINE_SysREG_READ__FUNC(_type, _name, _reg_name)	\
static inline _type read_ ## _name(void)					\
{															\
	_type v;												\
	__asm__ volatile ("mrs %0, " #_reg_name : "=r" (v));	\
	return v;												\
}

#define _DEFINE_SysREG_WRITE_FUNC(_type, _name, _reg_name)	\
static inline void write_ ## _name(_type v)					\
{															\
	__asm__ volatile ("msr " #_reg_name ", %0" : : "r" (v));\
}



// 通用寄存器访问相关定义
#define DEFINE_GenREG_RW_FUNCS(_type, _name)				\
	_DEFINE_GenREG_READ__FUNC (_type, _name, _name)			\
	_DEFINE_GenREG_WRITE_FUNC (_type, _name, _name)

#define _DEFINE_GenREG_READ__FUNC(_type, _name, _reg_name)	\
static inline _type read_ ## _name(void)					\
{															\
	_type v;												\
	__asm__ volatile ("mov %0, " #_reg_name : "=r" (v));	\
	return v;												\
}

#define _DEFINE_GenREG_WRITE_FUNC(_type, _name, _reg_name)	\
static inline void write_ ## _name(_type v)					\
{															\
	__asm__ volatile ("mov " #_reg_name ", %0" : : "r" (v));\
}



// CPU Inerface 寄存器的相关定义
#define DEFINE_CPUIFR_RW_FUNCS(_type, _name, _reg_name)		\
	_DEFINE_CPUIFR_READ__FUNC (_type, _name, _reg_name)		\
	_DEFINE_CPUIFR_WRITE_FUNC (_type, _name, _reg_name)

#define _DEFINE_CPUIFR_READ__FUNC(_type, _name, _reg_name)	\
static inline _type read_ ## _name(void)					\
{															\
	_type v;												\
	__asm__ volatile ("mrs %0, " #_reg_name : "=r" (v));	\
	return v;												\
}

#define _DEFINE_CPUIFR_WRITE_FUNC(_type, _name, _reg_name)	\
static inline void write_ ## _name(_type v)					\
{															\
	__asm__ volatile ("msr " #_reg_name ", %0" : : "r" (v));\
}

// mrc 访问协处理器的寄存器的相关定义

//}


//{ assert 定义
/**********************************************************/
/*************************  assert  ***********************/
/**********************************************************/
#if Use_ASSERTIONS
	static inline void do_assert(u8* errFileName, u32 errLine, u8* errMsg)
	{
		printf("\nASSERT FAILED!\n");
		printf("  File: %s\n", errFileName);
		printf("  Line: %d\n", errLine);
		printf("  Expr: %s\n", errMsg);

		while (1);  // 死循环停机
	}
	#define assert(e)	((e) ? (void)0 : do_assert(__FILE__, __LINE__, #e))
#else
	#define assert(e)	((void)0)
#endif

#define Need assert
#define need assert
#define NEED assert
//}


#define BROMA_GOTO_NEXT	0
#define BROMA_RELOAD_ME	1
void back_to_bootrom(int exitCode);

#endif

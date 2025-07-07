.text

	.global setGenTimerFreq
setGenTimerFreq:
	MSR		CNTFRQ_EL0, X0
	RET

	.global getPhyCount
getPhyCount:
	MRS		X0, CNTPCT_EL0
	RET

	.global setEL3PhyTimerCV
setEL3PhyTimerCV:
	MSR		CNTPS_CVAL_EL1, X0
	RET

	.global setEL3PhyTimerCtrl
setEL3PhyTimerCtrl:
	MSR CNTPS_CTL_EL1, X0
	RET



/*******************************************************************************************************/
//{ 保留！

.macro	switch_el, xreg, el3_label, el2_label, el1_label
	mrs	\xreg, CurrentEL
	cmp	\xreg, 0xc
	b.eq	\el3_label
	cmp	\xreg, 0x8
	b.eq	\el2_label
	cmp	\xreg, 0x4
	b.eq	\el1_label
.endm


/*******************************************************/
//ref:D:u-boot/arch/arm/cpu/armv8/exceptions.S

/* Enter Exception.
 * This will save the processor state that is ELR/X0~X30 to the stack frame.*/
 
.macro	EXSaveContex
	stp	x29, x30, [sp, #-16]!
	stp	x27, x28, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x21, x22, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	stp	x17, x18, [sp, #-16]!
	stp	x15, x16, [sp, #-16]!
	stp	x13, x14, [sp, #-16]!
	stp	x11, x12, [sp, #-16]!
	stp	x9,  x10, [sp, #-16]!
	stp	x7,  x8,  [sp, #-16]!
	stp	x5,  x6,  [sp, #-16]!
	stp	x3,  x4,  [sp, #-16]!
	stp	x1,  x2,  [sp, #-16]!

	/* Could be running at EL3/EL2/EL1 */
	switch_el x11, 3f, 2f, 1f
3:
	mrs	x2, elr_el3
	b	0f
2:
	mrs	x2, elr_el2
	b	0f
1:
	mrs	x2, elr_el1
0:
	stp	x2, x0, [sp, #-16]!
.endm

/*
 * Exit Exception.
 * This will restore the processor state that is ELR/X0~X30
 * from the stack frame.
 */
.macro	EXRestoreContex
	ldp	x2, x0, [sp],#16

	/* Could be running at EL3/EL2/EL1 */
	switch_el x11, 3f, 2f, 1f
3:	msr	elr_el3, x2
	b	0f
2:	msr	elr_el2, x2
	b	0f
1:	msr	elr_el1, x2
0:
	ldp	x1,  x2,  [sp],#16
	ldp	x3,  x4,  [sp],#16
	ldp	x5,  x6,  [sp],#16
	ldp	x7,  x8,  [sp],#16
	ldp	x9,  x10, [sp],#16
	ldp	x11, x12, [sp],#16
	ldp	x13, x14, [sp],#16
	ldp	x15, x16, [sp],#16
	ldp	x17, x18, [sp],#16
	ldp	x19, x20, [sp],#16
	ldp	x21, x22, [sp],#16
	ldp	x23, x24, [sp],#16
	ldp	x25, x26, [sp],#16
	ldp	x27, x28, [sp],#16
	ldp	x29, x30, [sp],#16
	eret
.endm


/* Exception vectors.*/
	.align	11
.globl vectors
vectors:
	// 第一组
	.align	7
	b	_do_bad_sync	    //Current EL Synchronous Thread  
                          
	.align	7              
	b	_do_bad_irq	        //Current EL IRQ Thread  
                            
	.align	7               
	b	_do_bad_fiq	        //Current EL FIQ Thread 
                            
	.align	7               
	b	_do_bad_error	    //Current EL Error Thread 
                           

	// 第二组
	.align	7               
	b	_do_sync	        //Current EL Synchronous Handler 
                            
	.align	7               
	b	_do_irq		        //Current EL IRQ Handler 
                            
	.align	7               
	b	_do_fiq		        //Current EL FIQ Handler 
                           
	.align	7              
	b	_do_error	        //Current EL Error Handler 


// 第一组
_do_bad_sync:
	EXSaveContex
	b	. 				//do_bad_sync

_do_bad_irq:
	EXSaveContex
	b	. 				//do_bad_irq

_do_bad_fiq:
	EXSaveContex
	b	.				//do_bad_fiq

_do_bad_error:
	EXSaveContex
	b	. 				//do_bad_error

// 第二组
_do_sync:
	EXSaveContex
	bl	SYNHandler      //SYNHandler
	EXRestoreContex

_do_irq:
	EXSaveContex
	bl	irqHandler      //irqHandler
	EXRestoreContex

_do_fiq:
	EXSaveContex
	bl	fiqHandler      //fiqHandler
	EXRestoreContex

_do_error:
	EXSaveContex
	bl	ErrHandler      //ErrHandler
	EXRestoreContex



//}

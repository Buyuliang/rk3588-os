.data
	.global BootromContex
	BootromContex:
		.zero 8*13

.text
.global  _start
_start:

	b save_boot_params
	.global	save_boot_params_ret
	save_boot_params_ret:

	//ldr	x0, =24000000           //24MHz
	//msr	cntfrq_el0, x0			//Initialize crystal oscillator

	bl clean_bss
clean_bss_complete:

	b tpl_main

halt:
	b .

/*******************************************************/	
clean_bss:
	ldr x0, =__bss_start    //bss start
	ldr x1, =__bss_end      //bss end

	memClear_loop:  
	str xzr, [x0] 
	add	x0, x0, #8	
	cmp x0, x1          
	b.lo memClear_loop        
	
	b clean_bss_complete


.global	setjmp
setjmp:
	/* 保存上下文 */
	stp  x19, x20, [x0,#0]
	stp  x21, x22, [x0,#16]
	stp  x23, x24, [x0,#32]
	stp  x25, x26, [x0,#48]
	stp  x27, x28, [x0,#64]
	stp  x29, x30, [x0,#80]
	mov  x2, sp
	str  x2, [x0, #96]
	mov  x0, #0
	ret

.global	longjmp
longjmp:
	ldp  x19, x20, [x0,#0]
	ldp  x21, x22, [x0,#16]
	ldp  x23, x24, [x0,#32]
	ldp  x25, x26, [x0,#48]
	ldp  x27, x28, [x0,#64]
	ldp  x29, x30, [x0,#80]
	ldr  x2, [x0,#96]
	mov  sp, x2
	/* 恢复上下文, 第二个参数做返回值，如果第二个参数是0就加1返回1  */
	adds x0, xzr, x1
	csinc x0, x0, xzr, ne
	/* invalid icache for cortex a35 */
	//branch_if_a35_core x1, __asm_invalidate_icache_all
	ret


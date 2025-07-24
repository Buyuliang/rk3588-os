.section .data
    .equ CONFIG_DEBUG_UART_BASE, 0xfeb50000   // UART 基地址

    // 定义测试字符串
    test_str: .asciz "Hello, TOM OS!\r\n"
    separator_str: .asciz "=================\r\n"

.section .text
    .global _start
_start:
    // 初始化栈指针
    ldr     x0, =stack_top
    mov     sp, x0

    // 打印换行
    mov     x0, #'\r'
    bl      putc
    mov     x0, #'\n'
    bl      putc

    // 逐个字符打印
    mov     x0, #'A'
    bl      putc
    mov     x0, #'B'
    bl      putc
    mov     x0, #'C'
    bl      putc
    mov     x0, #'\r'
    bl      putc
    mov     x0, #'\n'
    bl      putc

    bl        main

// ---------------------------------------
// putc 函数：向UART发送一个字符
// 参数：
//   x0 = 要发送的字符
// 返回：
//   无
// ---------------------------------------
    .global putc
putc:
    stp     x29, x30, [sp, #-16]!    // 保存FP和LR
    mov     x29, sp                  // 设置FP
    
    ldr     x9, =CONFIG_DEBUG_UART_BASE

wait_uart_ready:
    ldr     w1, [x9, #0x14]         // LSR 偏移 0x14
    tst     w1, #0x20               // 检查 bit5 (THR 空)
    beq     wait_uart_ready

    // 写入字符到 THR
    strb    w0, [x9, #0x00]  

    ldp     x29, x30, [sp], #16     // 恢复FP和LR
    ret

//  // ---------------------------------------
//  // puts 函数：在 UART 上打印一个字符串
//  // 参数： 
//  //   x0 = 字符串地址（以NULL结尾）
//  // 返回：
//  //   无
//  // ---------------------------------------
//      .global puts
//  puts:
//      stp     x29, x30, [sp, #-16]!    // 保存FP和LR
//      mov     x29, sp                  // 设置FP
//      stp     x19, x20, [sp, #-16]!    // 保存被调用者保存寄存器
//      
//      mov     x19, x0                 // 保存字符串地址到x19
//      
//  puts_loop:
//      ldrb    w20, [x19], #1          // 加载字符并递增指针
//      cmp     w20, #0                 // 检查是否为字符串结束符
//      beq     puts_done
//      mov     x0, x20                 // 将字符放入x0
//      bl      putc
//      b       puts_loop
//      
//  puts_done:
//      ldp     x19, x20, [sp], #16     // 恢复被调用者保存寄存器
//      ldp     x29, x30, [sp], #16     // 恢复FP和LR
//      ret

    // ---------------------------------------
    // 栈区域
    .align 4
stack_bottom:
    .space 0x1000                    // 4KB栈空间
stack_top:
    .quad stack_bottom + 0x1000

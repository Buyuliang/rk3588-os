.section .data
    .global BootromContex
BootromContex:
    .zero 8*13

    .equ CONFIG_DEBUG_UART_BASE, 0xfeb50000   // UART 基地址（根据实际修改）

    // 定义字符串常量
    tom_os_str: .asciz "TOM OS\r\n\0a"
    separator_str: .asciz "=================\r\nabcde\r\n\0"
    boot_info_str: .asciz "Booting TOM OS v1.1\r\n\0a"
    error_str: .asciz "ERROR: "

.section .text
    .global _start
_start:

    // 初始化栈指针
    ldr     x0, =stack_top
    mov     sp, x0

    mov     x0, #'\r'
    bl      putc
    mov     x0, #'\n'
    bl      putc

    mov     x0, #'A'
    bl      putc

    mov     x0, #'l'
    bl      putc

    mov     x0, #'l'
    bl      putc

    mov     x0, #'\r'
    bl      putc
    mov     x0, #'\n'
    bl      putc

    // 打印分隔符
    ldr     x0, =separator_str
    bl      puts

    // 打印启动信息
    mov     x0, #0
    ldr     x0, =boot_info_str
    bl      puts

    // 打印分隔符
    mov     x0, #0
    ldr     x0, =separator_str
    bl      puts

    // 打印系统信息
    ldr     x0, =tom_os_str
    bl      puts

halt:
    b        halt




//// ---------------------------------------
//// putc 函数：在 UART 上打印一个字符
//// 参数： 
//// x0 = 要打印的字符 
//// x1 = 赋值的临时变量
//// x3 = 串口基地址
//// ---------------------------------------
//    .global putc
//putc:
//    ldr     x9, =CONFIG_DEBUG_UART_BASE
//
//wait_uart_ready:
//    ldr     w1, [x9, #0x14]         // LSR 偏移 0x14
//    tst     w1, #0x20               // 检查 bit5 (THR 空)
//    beq     wait_uart_ready
//
//    // 写入字符到 THR
//    strb    w0, [x9, #0x00]  
//    ret
//
//
//
//// ---------------------------------------
//// puts 函数：在 UART 上打印一个字符串
//// 参数： 
////   x2 = 字符串地址
////   x4 = 临时变量 字符串地址
////   x5 = 临时变量字符串
//// ---------------------------------------
//    .global puts
//puts:
//    mov     x4, x2                // 保存字符串地址到x19
//    
//puts_loop:
//    ldrb    w5, [x4], #1         // 加载字符并递增指针
//    cmp     w5, #0               // 检查是否为字符串结束符
//    beq     puts_done
//    mov     x0, x5               // 将字符放入x0 (使用64位寄存器)
//    bl      putc
//    b       puts_loop
//    
//puts_done:
//    mov     x0, #'X'                // 将字符放入x0 (使用64位寄存器)
//    bl      putc                   // 调用putc打印字符
//    ret


// ---------------------------------------
// putc 函数：向UART发送一个字符
// 参数：
//   x0 = 要发送的字符
// 返回：
//   无
// ---------------------------------------
    .global putc
putc:
 //   stp     x29, x30, [sp, #-16]!    // 保存FP和LR
 //   mov     x29, sp                  // 设置FP
    
    ldr     x9, =CONFIG_DEBUG_UART_BASE

wait_uart_ready:
    ldr     w1, [x9, #0x14]         // LSR 偏移 0x14
    tst     w1, #0x20               // 检查 bit5 (THR 空)
    beq     wait_uart_ready

    // 写入字符到 THR
    strb    w0, [x9, #0x00]  

 //   ldp     x29, x30, [sp], #16     // 恢复FP和LR
    ret

// ---------------------------------------
// puts 函数：在 UART 上打印一个字符串
// 参数： 
//   x0 = 字符串地址
// 返回：
//   无
// ---------------------------------------
    .global puts
puts:
    stp     x29, x30, [sp, #-16]!    // 保存FP和LR
    mov     x29, sp                  // 设置FP
    stp     x19, x20, [sp, #-16]!    // 保存被调用者保存寄存器
    
    mov     x19, x0                 // 保存字符串地址到x19
    
puts_loop:
    ldrb    w20, [x19], #1          // 加载字符并递增指针
    cmp     w20, #0                 // 检查是否为字符串结束符
    beq     puts_done
    mov     x0, x20                 // 将字符放入x0
    bl      putc
    b       puts_loop
    
puts_done:
    // 移除了额外的字符打印
    
    ldp     x19, x20, [sp], #16     // 恢复被调用者保存寄存器
    ldp     x29, x30, [sp], #16     // 恢复FP和LR
    ret


//    .global puts
//puts:
//    mov     x19, x2                // 保存字符串地址到x19
//    
//puts_loop:
//    ldrb    w20, [x19], #1         // 加载字符并递增指针
//    cmp     w20, #0               // 检查是否为字符串结束符
//    beq     puts_done
//    mov     x0, x20                // 将字符放入x0 (使用64位寄存器)
//    ldr     x21, =CONFIG_DEBUG_UART_BASE
//
//wait_uart_ready_1:
//    ldr     w1, [x21, #0x14]         // LSR 偏移 0x14
//    tst     w1, #0x20                // 检查 bit5 (THR 空)
//    beq     wait_uart_ready_1
//
//    // 写入字符到 THR
//    strb    w0, [x21, #0x00]
//    b       puts_loop
//    
//puts_done:
//    mov     x0, #'X'                // 将字符放入x0 (使用64位寄存器)
//    ldr     x21, =CONFIG_DEBUG_UART_BASE
//
//wait_uart_ready_2:
//    ldr     w1, [x21, #0x14]         // LSR 偏移 0x14
//    tst     w1, #0x20                // 检查 bit5 (THR 空)
//    beq     wait_uart_ready_2
//
//    // 写入字符到 THR
//    strb    w0, [x21, #0x00]
//    ret

// ---------------------------------------
// 打印错误信息
// 参数： x0 = 错误代码
// ---------------------------------------
print_error:
    // 保存寄存器
    stp     x0, x1, [sp, #-16]!
    
    // 打印错误前缀
    ldr     x0, =error_str
    bl      puts
    
    // 恢复寄存器
    ldp     x0, x1, [sp], #16

    mov     x0, #'\n'
    bl      putc
    
    ret

    // ---------------------------------------
    // 栈区域（实际项目中应在链接脚本中定义）
    .align 4
stack_bottom:
    .space 0x1000                    // 4KB栈空间
stack_top:
    .word stack_bottom + 0x1000

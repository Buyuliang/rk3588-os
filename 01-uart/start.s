.section .data
    .global BootromContex
BootromContex:
    .zero 8*13

    .equ CONFIG_DEBUG_UART_BASE, 0xfeb50000   // UART 基地址（根据实际修改）

    // 定义字符串常量
    tom_os_str: .asciz "TOM OS\r\n"
    separator_str: .asciz "=================\r\n\0"
    boot_info_str: .asciz "Booting TOM OS v1.0 \r\n\0"
    error_str: .asciz "ERROR: "

.section .text
    .global _start
_start:
    // 保存返回地址到栈上（如果被调用）
    stp     x30, x29, [sp, #-16]!

    // 禁用中断
    msr     daifset, #0xf

    // 初始化栈指针
    ldr     x0, =stack_top
    mov     sp, x0

    // 初始化BSS段（清零未初始化的全局变量）
    bl      init_bss

    // 打印分隔符
    ldr     x0, =separator_str
    bl      puts

    // 打印启动信息
    ldr     x0, =boot_info_str
    bl      puts

    // 打印分隔符
    ldr     x0, =separator_str
    bl      puts

    ///// 打印系统信息
    ///ldr     x0, =tom_os_str
    ///bl      puts

    //// 初始化时钟系统
    //bl      init_clock
//
    //// 初始化内存系统
    //bl      init_memory
//
    //// 初始化设备
    //bl      init_devices
//
    //// 保存启动参数
    //bl      save_boot_params

    // 如果跳转失败，进入无限循环
halt:
    // 恢复返回地址和帧指针
    //ldp     x30, x29, [sp], #16
    b        halt

    // ---------------------------------------
    // 初始化BSS段
init_bss:
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
    mov     x2, #0

init_bss_loop:
    cmp     x0, x1
    b.ge    init_bss_done
    str     x2, [x0], #8
    b       init_bss_loop

init_bss_done:
    ret

    // ---------------------------------------
    // 初始化时钟系统
init_clock:
    // 初始化系统时钟（示例代码）
    ldr     x0, =24000000           // 24MHz
    msr     cntfrq_el0, x0         // 初始化晶振频率
    ret

    // ---------------------------------------
    // 初始化内存系统
init_memory:
    // 初始化内存控制器（示例代码）
    // 实际代码会根据具体硬件实现
    ret

    // ---------------------------------------
    // 初始化设备
init_devices:
    // 初始化关键设备（示例代码）
    // 实际代码会根据具体硬件实现
    ret

    // ---------------------------------------
    // 保存启动参数
save_boot_params:
    // 保存启动参数到BootromContex
    // 实际代码会根据具体需求实现
    ret

    // ---------------------------------------
    // putc 函数：在 UART 上打印一个字符
    // 参数： x0 = 要打印的字符
    .global putc
putc:
    // 如果是 '\n'，先打印 '\r'
    // cmp     x0, #'\n'
    // b.ne    putc_check_uart_ready
    // mov     x1, #'\r'
    // bl      putc

putc_check_uart_ready:
    ldr     x20, =CONFIG_DEBUG_UART_BASE

wait_uart_ready:
    ldr     w1, [x20, #0x14]         // LSR 偏移 0x14
    tst     w1, #0x20                // 检查 bit5 (THR 空)
    beq     wait_uart_ready

    // 写入字符到 THR
    strb    w0, [x20, #0x00]
    ret

    // ---------------------------------------
    // puts 函数：在 UART 上打印一个字符串
    // 参数： x0 = 字符串地址
    .global puts
puts:
    stp     x19, x20, [sp, #-16]!  // 保存被调用者保存的寄存器
    mov     x19, x0                // 保存字符串地址到x19
    
puts_loop:
    ldrb    w20, [x19], #1         // 加载字符并递增指针
    cmp     w20, #0                // 检查是否为字符串结束符
    beq     puts_done
    mov     x0, x20                // 将字符放入x0 (使用64位寄存器)
    bl      putc                   // 调用putc打印字符
    b       puts_loop
    
puts_done:
    ldp     x19, x20, [sp], #16    // 恢复寄存器
    ret

    // ---------------------------------------
    // 打印错误信息
    // 参数： x0 = 错误代码
print_error:
    // 保存寄存器
    stp     x0, x1, [sp, #-16]!
    
    // 打印错误前缀
    ldr     x0, =error_str
    bl      puts
    
    // 恢复寄存器
    ldp     x0, x1, [sp], #16
    
    // 转换错误代码为ASCII并打印
    // 这里需要实现数字到字符串的转换
    // 简化起见，这里省略具体实现
    
    // 打印换行
    mov     x0, #'\n'
    bl      putc
    
    ret

    // ---------------------------------------
    // 栈区域（实际项目中应在链接脚本中定义）
    .align 4
stack_bottom:
    .space 0x1000                    // 4KB栈空间
stack_top:

## 1. ENTRY(_start)
```bash
    作用：指定程序的入口点（entry point）。

    _start 是程序执行时最先执行的函数或符号（通常是汇编中的启动代码）。

    链接器会在 ELF 文件头（ELF Header）中设置入口地址为 _start 符号的地址。

    运行时 CPU 会跳转到这里开始执行程序。
```

### 2. PHDRS { ... }
```bash
这是定义 ELF 的 程序头表（Program Headers）。

    PHDRS 定义了 ELF 文件的段加载信息，告诉操作系统如何将文件内容映射到内存。

具体内容：

text PT_LOAD FLAGS(5);   /* Read + Execute */
data PT_LOAD FLAGS(6);   /* Read + Write */

    text 和 data 是自定义的程序头段名称，下面的 SECTIONS 里会用到。

    PT_LOAD 表示这是一个“加载段”，告诉操作系统需要将其内容加载到内存。

    FLAGS(5) 和 FLAGS(6) 指定段的权限：

        5 = 0b101 = 读（R）+ 执行（X）

        6 = 0b110 = 读（R）+ 写（W）

    所以 text 段是可读可执行，data 段是可读可写。
```

### 3. SECTIONS { ... }

这是最核心部分，定义了链接器如何将输入的各个段映射到输出文件中的地址和分布。
#### . = 0x00000000;
```bash
    将当前段起始地址设置为 0x00000000。

    这是程序加载的基地址，通常裸机程序从 0 地址开始执行。
```

#### . = ALIGN(8);
```bash
    将当前位置按 8 字节对齐。

    确保后续段起始地址是 8 字节的整数倍，满足硬件对齐要求。
```

#### .text : { ... } :text
```bash
    定义一个段，名字是 .text，它对应于前面 PHDRS 中定义的 text 段。

    大括号 {} 内指定将哪些输入段（段是编译后目标文件或库中的区域）拷贝到输出段 .text。

具体内容：

KEEP(*(.text._start))
*(.text*)

    KEEP 表示确保 .text._start 段里的内容即使没有被引用也要保留，防止链接器优化删除启动代码。

    *(.text*) 匹配所有以 .text 开头的输入段，包含所有程序代码，放入 .text 段。

    :text 表示这个 .text 段属于程序头 text，即之前定义的 PT_LOAD 可执行加载段。
```

#### .rodata ALIGN(8) : { *(.rodata*) } :text
```bash
    定义 .rodata（只读数据段），也对齐 8 字节。

    拷贝所有输入段名匹配 .rodata* 的内容（常量字符串、只读数据等）。

    这个段也挂载在 text 程序头上，通常放在 .text 旁边，属于可执行段。
```

#### .data ALIGN(8) : { *(.data) } :data
```bash
    定义 .data 段，对齐 8 字节。

    拷贝所有 .data 输入段（程序中的全局变量，已初始化的可读写数据）。

    这个段挂载到 data 程序头上，具有读写权限。
```

#### . = ALIGN(8); __bss_start = .;
```bash
    先对齐 8 字节。

    定义符号 __bss_start，表示 .bss 段开始地址，用于程序运行时初始化。

.bss ALIGN(8) : { *(.bss) *(COMMON) } :data

    定义 .bss 段，对齐 8 字节。

    .bss 包含未初始化的全局变量。

    *(COMMON) 匹配编译器生成的“common”符号（未初始化的全局变量）。

    这个段也挂载到 data 程序头上，具有读写权限。
```

#### __bss_end = .;
```bash
    定义符号 __bss_end，表示 .bss 段结束地址，通常用于程序运行时清零。
```

### 总结

| 部分       | 含义                         |
|------------|------------------------------|
| ENTRY(_start) | 指定程序入口地址             |
| PHDRS      | 定义 ELF 程序头加载段，分配内存权限 |
| .text 段   | 存放代码和只读数据，放入 text 加载段 |
| .rodata 段 | 存放只读常量，放入 text 加载段   |
| .data 段   | 存放初始化的全局变量，放入 data 加载段 |
| .bss 段    | 存放未初始化的全局变量，放入 data 加载段 |
| KEEP()     | 防止重要代码被优化器删除       |
| ALIGN(8)   | 内存地址对齐，满足架构要求     |


注：
| 寄存器名     | 全称                       | 作用描述                                                                 | 常用位说明（位号）                                                                                                                                     |
|--------------|----------------------------|--------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------|
| VBAR_EL3     | Vector Base Address Register (EL3) | 设置异常向量表的基地址（用于 EL3 异常处理）                                       | 无具体位；整个寄存器用于存储异常向量表的地址。                                                                                                        |
| SCR_EL3      | Secure Configuration Register (EL3) | 配置 EL3 的安全状态、异常路由和执行状态等。                                                | - RW（位10）：设置异常返回的下一个 EL 是 AArch64（1）还是 AArch32（0）  <br> - EA（位3）：SError 是否路由到 EL3 <br> - FIQ（位2）：FIQ 路由到 EL3 <br> - IRQ（位1）：IRQ 路由到 EL3 <br> - NS（位0）：表示当前是否处于 Non-secure 状态 |
| CPTR_EL3     | Architectural Feature Trap Register (EL3) | 控制是否屏蔽对 SIMD/FP 寄存器的访问，通常设置为 0 表示不屏蔽。                              | - TCPAC（位31）：屏蔽 EL1/EL0 对浮点/高级SIMD寄存器的访问 <br> - TFP（位10）：屏蔽 EL1/EL0/EL2 对浮点/高级SIMD 的访问（通常设置为0启用）                                   |
| CNTFRQ_EL0   | Counter-timer Frequency register | 设置物理计数器的频率（通常是 SoC 的晶振频率）                                         | 整个寄存器是一个 32 位无符号值，表示每秒计数的频率，例如设置为 24000000 表示 24MHz                                                             |



#### CNTPS_CVAL_EL1 

CNTPS_CVAL_EL1 是 ARMv8 架构中 EL1 Exception Level 下，用于 Secure Physical Timer 的一个比较值（Compare Value）寄存器。它与 Secure Physical Timer 一起工作，用于定时中断的产生。

| 寄存器名         | CNTPS_CVAL_EL1 |
|------------------|----------------|
| 全称             | Counter-timer Physical Secure CompareValue register (EL1) |
| 作用             | 设置 Secure Physical Timer 触发中断的目标计数值 |
| 位数             | 64-bit |
| 使用模式         | EL1，Secure 状态可访问 |
| 关联寄存器       | CNTPS_CTL_EL1（控制），CNTPS_TVAL_EL1（偏移量） |
| 典型用途         | 配置 Secure Timer 在特定时间点产生中断 |
| 读取/写入指令示例 | `MRS X0, CNTPS_CVAL_EL1` / `MSR CNTPS_CVAL_EL1, X0` |

#### CNTPS_CTL_EL1

| 位段      | 名称        | 描述                                    |
| ------- | --------- | ------------------------------------- |
| \[0]    | `ENABLE`  | 设置为 1 启用 Secure EL1 定时器；设置为 0 禁用该定时器。 |
| \[1]    | `IMASK`   | 中断屏蔽位。<br>1：屏蔽定时器中断；<br>0：使能中断。       |
| \[2]    | `ISTATUS` | 中断状态位（只读）。<br>1：事件已经发生；<br>0：事件未发生。   |
| \[31:3] | —         | 保留，读取时为 0，写入时应忽略。                     |


#### GICv3 配置流程表格
| 步骤 | 模块              | 寄存器/接口                                | 描述                                      |
| -- | --------------- | ------------------------------------- | --------------------------------------- |
| 1  | 系统控制器           | SCR\_EL3                              | 设置 HCE/SMD/NS，允许 EL1/EL2 访问 GIC（启用中断管理） |
| 2  | CPU 接口（GICR）    | GICR\_CTLR                            | 启用 Redistributor，确保每个核心的中断分发器处于就绪状态     |
| 3  | CPU 接口（GICR）    | GICR\_TYPER                           | 获取当前 CPU 的 GICR 范围和中断范围                 |
| 4  | GIC Distributor | GICD\_CTLR                            | 启用 Distributor，全局使能中断分发                 |
| 5  | GIC Distributor | GICD\_IGROUPRn / GICD\_ISENABLERn     | 配置中断为 Group0 或 Group1，设置哪些中断使能          |
| 6  | GIC Distributor | GICD\_IPRIORITYRn                     | 设置中断优先级（8-bit）                          |
| 7  | GIC Distributor | GICD\_ITARGETSRn / GICR\_TYPER        | 配置目标 CPU，或 LPIs 目标 Redistributor        |
| 8  | Redistributor   | GICR\_ISENABLER0 等                    | 启用 SGIs 和 PPIs（每核本地中断）                  |
| 9  | CPU 接口系统寄存器     | ICC\_PMR\_EL1                         | 设置中断优先级掩码                               |
| 10 | CPU 接口系统寄存器     | ICC\_IGRPEN1\_EL1 / ICC\_IGRPEN0\_EL1 | 启用 Group0 和 Group1 中断处理                 |
| 11 | CPU 接口系统寄存器     | ICC\_BPR0\_EL1 / ICC\_BPR1\_EL1       | 配置中断优先级分组                               |
| 12 | 处理器中断处理流程       | ICC\_IAR1\_EL1 / ICC\_EOIR1\_EL1      | 获取中断 ID，完成后写入 EOI                       |

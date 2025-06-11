## UART

### lds 文件分析

#### 1. 位置计数器与对齐
```bash
. = 0x00000000;
. = ALIGN(8);
```
- .是链接器的位置计数器，代表当前输出文件中的位置。
- 0x00000000是程序的起始地址，这通常是嵌入式系统的做法。在 x86 系统中，可能会使用0x08048000（32 位）或0x400000（64 位）。
- ALIGN(8)确保位置计数器按 8 字节对齐，这有助于提高内存访问效率。

#### 2. 代码段（.text）
```bash
.text : 
{ 
	start.o(.text)
	*(.text) 
}
```
- .text是代码段，包含程序的机器指令。
- start.o(.text)表示首先放置start.o文件中的.text段。这通常是程序的入口点（如_start函数）。
- *(.text)表示收集所有其他目标文件中的.text段。

#### 3. 只读数据段（.rodata）
```bash
.rodata ALIGN(8) : {*(.rodata*)}
```
- .rodata包含只读数据，如字符串常量和只读全局变量。
- ALIGN(8)确保该段按 8 字节对齐。
- *(.rodata*)收集所有目标文件中以.rodata开头的段（如.rodata.cst4、.rodata.str1.1等）。

#### 4. 已初始化数据段（.data）
```bash
.data ALIGN(8) : { *(.data) }
```
- .data包含已初始化的全局变量和静态变量。
- ALIGN(8)确保该段按 8 字节对齐。

#### 5. 未初始化数据段（.bss）
```bash
. = ALIGN(8);
__bss_start = .;
.bss ALIGN(8) : { *(.bss) *(COMMON) }
__bss_end = .;
```
- .bss包含未初始化的全局变量和静态变量，在内存中只占用空间但不占用文件空间。
- __bss_start和__bss_end是用户定义的符号，分别标记.bss段的起始和结束位置。这些符号通常用于程序启动时清零.bss段。
- *(COMMON)收集所有未初始化的全局变量（COMMON 块）。

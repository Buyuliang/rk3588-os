ENTRY(_start)

PHDRS
{
  text PT_LOAD FLAGS(5);   /* Read + Execute */
  data PT_LOAD FLAGS(6);   /* Read + Write */
}

SECTIONS
{
  . = 0x00000000;
  . = ALIGN(8);

  .text : {
    KEEP(*(.text._start))  /* 保证入口函数被保留 */
    *(.text*)
  } :text

  .rodata ALIGN(8) : {
    *(.rodata*)
  } :text

  .data ALIGN(8) : {
    *(.data)
  } :data

  . = ALIGN(8);
  __bss_start = .;
  .bss ALIGN(8) : {
    *(.bss)
    *(COMMON)
  } :data
  __bss_end = .;
}

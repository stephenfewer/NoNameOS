OUTPUT_FORMAT("binary")
ENTRY(_main)
phys = 0x00000000;
virt = 0x10000000;
SECTIONS
{
  .text virt : AT( phys )
  {
    *(.text)
    . = ALIGN(4);
    *(.data)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
  	*(.bss)
    . = ALIGN(4);
  }
}

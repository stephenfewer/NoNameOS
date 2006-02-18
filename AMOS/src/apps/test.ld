OUTPUT_FORMAT("binary")
ENTRY(_main)
phys = 0x00000000;
virt = 0x10000000;
SECTIONS
{
  .text virt : AT( phys )
  {
    *(.text)
    . = ALIGN(16);
  }
  .data : AT( ADDR(.text) + SIZEOF(.text) )
  {
    *(.data)
    . = ALIGN(16);
  }
  .bss : AT( ADDR(.text) + SIZEOF(.text) + SIZEOF(.data) )
  {
  	bss = .;
  	*(.bss)
    . = ALIGN(16);
    *(COMMON)
    . = ALIGN(16);
  }
}

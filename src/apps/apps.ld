OUTPUT_FORMAT("binary")
ENTRY(main)
phys = 0x00000000;
virt = 0x10000000;
SECTIONS
{
  .text virt : AT( phys )
  {
    *(.text.entrypoint);
    *(.text.*);
    . = ALIGN(16);
    *(.data);
    . = ALIGN(16); 
	*(.rodata);
    . = ALIGN(16);
    *(.rodata.*);
    . = ALIGN(16);
    *(.eh_frame);
    . = ALIGN(16);   
    *(COMMON);
    . = ALIGN(16);
  	*(.bss);
    . = ALIGN(16);
  }
}

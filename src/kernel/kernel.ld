OUTPUT_FORMAT("elf32-i386")
ENTRY(setup)
phys = 0x00100000;
virt = 0xC0001000;
SECTIONS
{
  .setup phys : AT( phys )
  {
    setup = .;
    *(.setup)
    . = ALIGN(4096);
  }
  .kernel virt : AT( ADDR(.setup) + SIZEOF(.setup) )
  {
    kernel = .;
    *(.kernel)
    *(.text)
    *(.text.*)
	. = ALIGN(4096);
  }
  .data : AT( ADDR(.setup) + SIZEOF(.setup) + SIZEOF(.kernel) )
  {
    data = .;
    *(.data)
	. = ALIGN(16);
	*(.rodata)
	. = ALIGN(16);
	*(.rodata.*)
	. = ALIGN(16);
	*(.eh_frame)
    . = ALIGN(4096);
  }
  .bss : AT( ADDR(.setup) + SIZEOF(.setup) + SIZEOF(.kernel) + SIZEOF(.data) )
  {
  	bss = .;
  	*(.bss)
    . = ALIGN(16);
    *(COMMON)
    . = ALIGN(4096);
  }
  end = .;
}

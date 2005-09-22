#include <kernel/gdt.h>
#include <kernel/kernel.h>

struct GDT_ENTRY	gdt_table[GDT_ENTRYS];

struct GDT_POINTER	gdt_pointer;

void gdt_setEntry( int i, DWORD base, DWORD limit, BYTE access, BYTE granularity )
{
    gdt_table[i].base_low = (base & 0xFFFF);

    gdt_table[i].base_middle = (base >> 16) & 0xFF;

    gdt_table[i].base_high = (base >> 24) & 0xFF;

    gdt_table[i].limit_low = (limit & 0xFFFF);

    gdt_table[i].granularity = ( (limit >> 16) & 0x0F );

    gdt_table[i].granularity |= (granularity & 0xF0);

    gdt_table[i].access = access;
}

void gdt_init()
{
    gdt_pointer.limit = ( sizeof(struct GDT_ENTRY) * GDT_ENTRYS ) - 1;
    // base should be physical?
    gdt_pointer.base = (unsigned int)&gdt_table;

	memset( (BYTE *)&gdt_table, 0x00, sizeof(struct GDT_ENTRY) * GDT_ENTRYS );

    // NULL descriptor
    gdt_setEntry( 0, 0L, 0L, 0x00, 0x00 );

	// code segment
    gdt_setEntry( 1, 0L, 0xFFFFFFFF, 0x9A, 0xCF );

	// data segment
    gdt_setEntry( 2, 0L, 0xFFFFFFFF, 0x92, 0xCF );
}


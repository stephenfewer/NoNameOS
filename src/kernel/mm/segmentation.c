/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <kernel/mm/segmentation.h>
#include <kernel/kernel.h>
#include <lib/libc/string.h>

struct SEGMENTATION_GDT_ENTRY segmentation_gdt[SEGMENTATION_GDT_ENTRYS];

struct SEGMENTATION_GDT_POINTER segmentation_gdtp;

void segmentation_setEntry( int selector, DWORD base, DWORD limit, BYTE access, BYTE granularity )
{
	selector = SELECTOR_TO_INDEX(selector);
	
    segmentation_gdt[selector].base_low		= (base & 0xFFFF);

    segmentation_gdt[selector].base_middle	= (base >> 16) & 0xFF;

    segmentation_gdt[selector].base_high	= (base >> 24) & 0xFF;

    segmentation_gdt[selector].limit_low	= (limit & 0xFFFF);

    segmentation_gdt[selector].granularity	= ( (limit >> 16) & 0x0F );

    segmentation_gdt[selector].granularity	|= (granularity & 0xF0);

    segmentation_gdt[selector].access		= access;
}

void segmentation_ltr( WORD selector )
{
	// load the task regiter with the TSS in the given selector
	ASM( "ltr %0" :: "rm" (selector) );
}

void segmentation_reload( void )
{
	// load a linear address
	ASM( "lgdt (%0)" :: "r" ( &segmentation_gdtp ) );
	ASM( "movw %0, %%ax" :: "i" (KERNEL_DATA_SEL) );
	ASM( "movw %ax, %ds" );
	ASM( "movw %ax, %ss" );
	ASM( "movw %ax, %es" );
	ASM( "movw %ax, %fs" );
	ASM( "movw %ax, %gs" );
	ASM( "ljmp %0, $1f" :: "i" (KERNEL_CODE_SEL)  );	
	ASM( "1:" );
}

int segmentation_init( void )
{
    segmentation_gdtp.limit = ( sizeof(struct SEGMENTATION_GDT_ENTRY) * SEGMENTATION_GDT_ENTRYS ) - 1;
    // linear address, should be aligned on an 8byte boundry for best performance (3.5.1)
    segmentation_gdtp.base = (struct SEGMENTATION_GDT_ENTRY *)&segmentation_gdt;

	// clear the strucure
	memset( (void *)&segmentation_gdt, 0x00, sizeof(struct SEGMENTATION_GDT_ENTRY) * SEGMENTATION_GDT_ENTRYS );

    // NULL descriptor
    segmentation_setEntry( KERNEL_NULL_SEL, 0x00000000, 0x00000000, 0x00, 0x00 );

	// kernel code segment: ring0, read, execute
    segmentation_setEntry( KERNEL_CODE_SEL, 0x00000000, 0xFFFFFFFF, 0x9A, 0xCF );

	// kernel data segment: ring0, read, write
    segmentation_setEntry( KERNEL_DATA_SEL, 0x00000000, 0xFFFFFFFF, 0x92, 0xCF );

	// user code segment: ring3, read, execute
    segmentation_setEntry( USER_CODE_SEL,   0x00000000, 0xFFFFFFFF, 0xFA, 0xCF );

	// user data segment: ring3, read, write
    segmentation_setEntry( USER_DATA_SEL,   0x00000000, 0xFFFFFFFF, 0xF2, 0xCF );
    
    // empty descriptor, we fill it in with a TSS descriptor later in scheduler_init()
    segmentation_setEntry( KERNEL_TSS_SEL,  0x00000000, 0x00000000, 0x00, 0x00 );
	
	// Enable flat segmentation...
	segmentation_reload();
	
	return SUCCESS;
}

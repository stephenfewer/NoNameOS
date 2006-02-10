/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    Contact: steve [AT] harmonysecurity [DOT] com
 *    Web:     http://amos.harmonysecurity.com/
 *    License: GNU General Public License (GPL)
 */

#include <kernel/mm/segmentation.h>
#include <kernel/kernel.h>
#include <kernel/lib/string.h>

struct SEGMENTATION_GDT_ENTRY segmentation_gdt[SEGMENTATION_GDT_ENTRYS];

struct SEGMENTATION_GDT_POINTER segmentation_gdtp;

void segmentation_setEntry( int selector, DWORD base, DWORD limit, BYTE access, BYTE granularity )
{
	selector = SELECTOR_TO_INDEX(selector);
	
    segmentation_gdt[selector].base_low = (base & 0xFFFF);

    segmentation_gdt[selector].base_middle = (base >> 16) & 0xFF;

    segmentation_gdt[selector].base_high = (base >> 24) & 0xFF;

    segmentation_gdt[selector].limit_low = (limit & 0xFFFF);

    segmentation_gdt[selector].granularity = ( (limit >> 16) & 0x0F );

    segmentation_gdt[selector].granularity |= (granularity & 0xF0);

    segmentation_gdt[selector].access = access;
}

void segmentation_ltr( WORD selector )
{
	// load the task regiter with the TSS in the given selector
	ASM( "ltr %0" : : "rm" (selector) );
}

void segmentation_reload( void )
{
	ASM( "lgdt (%0)" :: "r" ( &segmentation_gdtp ) );
	ASM( "movw $0x10, %ax" );
	ASM( "movw %ax, %ds" );
	ASM( "movw %ax, %ss" );
	ASM( "movw %ax, %es" );
	ASM( "movw %ax, %fs" );
	ASM( "movw %ax, %gs" );
	ASM( "ljmp $0x08, $flush" );	
	ASM( "flush:" );
}

void segmentation_init()
{
    segmentation_gdtp.limit = ( sizeof(struct SEGMENTATION_GDT_ENTRY) * SEGMENTATION_GDT_ENTRYS ) - 1;
    // base should be physical?
    segmentation_gdtp.base = (unsigned int)&segmentation_gdt;

	memset( (void *)&segmentation_gdt, 0x00, sizeof(struct SEGMENTATION_GDT_ENTRY) * SEGMENTATION_GDT_ENTRYS );

    // NULL descriptor
    segmentation_setEntry( KERNEL_NULL_SEL, 0L, 0L, 0x00, 0x00 );

	// code segment
    segmentation_setEntry( KERNEL_CODE_SEL, 0L, 0xFFFFFFFF, 0x9A, 0xCF );

	// data segment
    segmentation_setEntry( KERNEL_DATA_SEL, 0L, 0xFFFFFFFF, 0x92, 0xCF );
    
    // empty descriptor, we fill it in with a TSS descriptor later in scheduler_init()
    segmentation_setEntry( KERNEL_TSS_SEL, 0L, 0L, 0x00, 0x00 );
	
	// Enable flat segmentation...
	segmentation_reload();
}

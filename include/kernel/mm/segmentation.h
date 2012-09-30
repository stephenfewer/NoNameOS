#ifndef _KERNEL_MM_SEGMENTATION_H_
#define _KERNEL_MM_SEGMENTATION_H_

#include <sys/types.h>

#define SEGMENTATION_GDT_ENTRYS		6

#define SELECTOR_TO_INDEX(selector)	(selector/8)

#define KERNEL_NULL_SEL				0x00
#define KERNEL_CODE_SEL				0x08
#define KERNEL_DATA_SEL				0x10
#define USER_CODE_SEL				0x18
#define USER_DATA_SEL				0x20
#define KERNEL_TSS_SEL				0x28

struct SEGMENTATION_GDT_ENTRY
{
    WORD limit_low;
    WORD base_low;
    BYTE base_middle;
    BYTE access;
    BYTE granularity;
    BYTE base_high;
} PACKED;

struct SEGMENTATION_GDT_POINTER
{
    WORD limit;
    struct SEGMENTATION_GDT_ENTRY * base;
} PACKED;

struct SEGMENTATION_TSS 
{
	WORD previous_tasklink, reserved0;
	DWORD esp0;
	WORD ss0, reserved1;
	DWORD esp1;
	WORD ss1, reserved2;
	DWORD esp2;
	WORD ss2, reserved3;
	struct PAGE_DIRECTORY * cr3;
	DWORD eip, eflags;
	DWORD eax, ecx, edx, ebx;
	DWORD esp, ebp, esi, edi;
	WORD es, reserved4, cs, reserved5;
	WORD ss, reserved6, ds, reserved7;
	WORD fs, reserved8, gs, reserved9;
	WORD ldt, reserved10;
	WORD reserved11, io_map_base;
};

void segmentation_setEntry( int, DWORD, DWORD, BYTE, BYTE );

void segmentation_ltr( WORD );

void segmentation_reload( void );

int segmentation_init( void );

#endif


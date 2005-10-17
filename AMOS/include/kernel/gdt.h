#ifndef _KERNEL_GDT_H_
#define _KERNEL_GDT_H_

#include <sys/types.h>

#define GDT_ENTRYS	4

#define SELECTOR_TO_INDEX(selector)	(selector/8)

#define KERNEL_NULL_SEL		0x00
#define KERNEL_CODE_SEL		0x08
#define KERNEL_DATA_SEL		0x10
#define KERNEL_TSS_SEL		0x18

struct GDT_ENTRY
{
    WORD limit_low;
    WORD base_low;
    BYTE base_middle;
    BYTE access;
    BYTE granularity;
    BYTE base_high;
} PACKED;

struct GDT_POINTER
{
    WORD limit;
    unsigned int   base;
} PACKED;

void gdt_setEntry( int, DWORD, DWORD, BYTE, BYTE );

void gdt_init();

#endif


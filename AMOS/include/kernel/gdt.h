#ifndef _KERNEL_GDT_H_
#define _KERNEL_GDT_H_

#include <sys/types.h>

#define GDT_ENTRYS	3

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

void gdt_init();

#endif


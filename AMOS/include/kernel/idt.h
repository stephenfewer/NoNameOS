#ifndef _KERNEL_IDT_H_
#define _KERNEL_IDT_H_

#include <sys/types.h>

#define IDT_ENTRYS				256

struct IDT_ENTRY
{
    WORD  base_low;
    WORD  selector;
    BYTE  reserved;
    BYTE  flags;
    WORD  base_high;
} PACKED;

struct IDT_POINTER
{
    WORD limit;
    unsigned int base;
} PACKED;

typedef void ( * ISR )();

extern void isr00();
extern void isr01();
extern void isr02();
extern void isr03();
extern void isr04();
extern void isr05();
extern void isr06();
extern void isr07();
extern void isr08();
extern void isr09();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

void idt_setEntry( BYTE, ISR, WORD, BYTE );

void idt_init();

#endif


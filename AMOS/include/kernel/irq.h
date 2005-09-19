#ifndef _KERNEL_IRQ_H_
#define _KERNEL_IRQ_H_

#include <kernel/idt.h>

#define IRQ_ENTRYS	16

extern void irq00();
extern void irq01();
extern void irq02();
extern void irq03();
extern void irq04();
extern void irq05();
extern void irq06();
extern void irq07();
extern void irq08();
extern void irq09();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

void irq_init( void );

#endif


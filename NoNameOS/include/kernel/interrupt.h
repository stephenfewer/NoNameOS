#ifndef _KERNEL_INTERRUPT_H_
#define _KERNEL_INTERRUPT_H_

#include <sys/types.h>
#include <kernel/pm/process.h>

#define INTERRUPT_TABLE_ENTRYS		256

#define INTERRUPT_PIC_1				0x20
#define INTERRUPT_PIC_2				0xA0

#define INTERRUPT_PIT_TIMER_0		0x40
#define INTERRUPT_PIT_TIMER_1		0x41
#define INTERRUPT_PIT_TIMER_2		0x42
#define INTERRUPT_PIT_COMMAND_REG	0x43

#define INTERRUPT_EOI				0x20

#define interrupt_enableAll()		ASM( "sti" ::: "memory" )

#define interrupt_disableAll()		ASM( "cli" ::: "memory" )

struct INTERRUPT_TABLE_ENTRY
{
    WORD base_low;
    WORD selector;
    BYTE reserved:5;
	BYTE unknown:3;
	BYTE size:5;
	BYTE DPL:2;
	BYTE present:1;
    WORD base_high;
} PACKED;

struct INTERRUPT_TABLE_POINTER
{
    WORD limit;
    DWORD base;
} PACKED;

typedef void (* INTERRUPT_SERVICE_ROUTINE)();

typedef struct PROCESS_INFO * (* INTERRUPT_HANDLER)( struct PROCESS_INFO * );

enum { 
	INT0 = 0,
	INT1,
	INT2,
	INT3,
	INT4,
	INT5,
	INT6,
	INT7,
	INT8,
	INT9,
	INT10,
	INT11,
	INT12,
	INT13,
	INT14,
	INT15,
	INT16,
	INT17,
	INT18,
	INT19,
	INT20,
	INT21,
	INT22,
	INT23,
	INT24,
	INT25,
	INT26,
	INT27,
	INT28,
	INT29,
	INT30,
	INT31,
	IRQ0,
	IRQ1,
	IRQ2,
	IRQ3,
	IRQ4,
	IRQ5,
	IRQ6,
	IRQ7,
	IRQ8,
	IRQ9,
	IRQ10,
	IRQ11,
	IRQ12,
	IRQ13,
	IRQ14,
	IRQ15,
	SYSCALL_INTERRUPT
};

extern void disable_intA();
extern void disable_intB();
extern void disable_irqA();
extern void disable_irqB();

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
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();
extern void isr48();

struct PROCESS_INFO * interrupt_dispatcher( struct PROCESS_INFO * process );

int interrupt_enable( int, INTERRUPT_HANDLER, BYTE );

int interrupt_disable( int );

int interrupt_init( void );

#endif


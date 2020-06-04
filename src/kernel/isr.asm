;//     AAA    M M    OOO    SSSS
;//    A   A  M M M  O   O  S 
;//    AAAAA  M M M  O   O   SSS
;//    A   A  M   M  O   O      S
;//    A   A  M   M   OOO   SSSS 
;//
;//    Author:  Stephen Fewer
;//    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
;//    Web:     http://www.harmonysecurity.com/
;//    License: GNU General Public License (GPLv3)

BITS 32

KERNEL_DATA_SEL			equ	0x10
USER					equ	0x01

EXTERN interrupt_dispatcher, scheduler_tss

GLOBAL disable_intA,disable_intB, disable_irqA, disable_irqB

%IMACRO ISR_A 1
GLOBAL	isr%1
ALIGN	4
isr%1:
	push dword 0
	push dword %1
	jmp isr_common
%ENDMACRO

%IMACRO ISR_B 1
GLOBAL	isr%1
ALIGN	4
isr%1:
	push dword %1
	jmp isr_common
%ENDMACRO

SECTION .kernel
ALIGN	4
isr_common:
    pushad								;// push all general purpose registers
    push ds								;// push al the segments
    push es
    push fs
    push gs 
    mov ax, KERNEL_DATA_SEL				;// set data segments for kernel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, dr0						;// EAX = the current process
    mov dword [eax], esp				;// save the current processes esp
    push eax
    call interrupt_dispatcher			;// call out C interrupt_dispatcher() function
   	pop ebx								;// EBX = the process that just called the dispatcher
	test ebx, eax						;// if we return the same process perform no context switch
	je noswitch
	mov dr0, eax						;// DR0 = the new current process
    cmp dword [eax+8], USER				;// if( process->privilege == USER )
    jne notss
    mov ebx, [scheduler_tss]			;// EBX = &scheduler_tss
    mov word [ebx+8], KERNEL_DATA_SEL	;// scheduler_tss->ss0 = KERNEL_DATA_SEL;
    mov ecx, [eax+12]					;// ECX = process->kstack_base
	add ecx, 4096
    mov dword [ebx+4], ecx				;// scheduler_tss->esp0 = (process->kstack_base + 4096)
notss:
    mov esp, [eax]						;// esp = process->kstack
    mov ebx, [eax+4]					;// switch over to the processes address space
    mov cr3, ebx						;// CR3 = process->page_dir
noswitch:
    pop gs								;// pop al the segments
    pop fs
    pop es
    pop ds
    popad								;// pop all general purpose registers
    add esp, 8							;// clean up the two dwords we pushed on via the _isrXX routine
    iret								;// iret back

disable_intA:
	iret

disable_intB:
	add esp, 4							;// clear the error code
	iret
	
disable_irqA:
	mov al, 0x20
	out 0x20, al
	iret

disable_irqB:
	mov al, 0x20
	out 0xA0, al
	out 0x20, al
	iret

ISR_A 00
ISR_A 01
ISR_A 02
ISR_A 03
ISR_A 04
ISR_A 05
ISR_A 06
ISR_A 07
ISR_B 08
ISR_A 09
ISR_B 10
ISR_B 11
ISR_B 12
ISR_B 13
ISR_B 14
ISR_A 15
ISR_A 16
ISR_A 17
ISR_A 18
ISR_A 19
ISR_A 20
ISR_A 21
ISR_A 22
ISR_A 23
ISR_A 24
ISR_A 25
ISR_A 26
ISR_A 27
ISR_A 28
ISR_A 29
ISR_A 30
ISR_A 31
ISR_A 32
ISR_A 33
ISR_A 34
ISR_A 35
ISR_A 36
ISR_A 37
ISR_A 38
ISR_A 39
ISR_A 40
ISR_A 41
ISR_A 42
ISR_A 43
ISR_A 44
ISR_A 45
ISR_A 46
ISR_A 47
ISR_A 48

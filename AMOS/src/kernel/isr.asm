;     AAA    M M    OOO    SSSS
;    A   A  M M M  O   O  S 
;    AAAAA  M M M  O   O   SSS
;    A   A  M   M  O   O      S
;    A   A  M   M   OOO   SSSS 
;
;    Author:  Stephen Fewer
;    Contact: steve [AT] harmonysecurity [DOT] com
;    Web:     http://amos.harmonysecurity.com/
;    License: GNU General Public License (GPL)

BITS 32

KERNEL_DATA_SEL			equ	0x10

EXTERN _interrupt_dispatcher, _scheduler_handler, _current_esp, _current_cr3

GLOBAL _disable_int, _disable_irqA, _disable_irqB, _isr32

%IMACRO ISR_A 1
GLOBAL	_isr%1
ALIGN	4
_isr%1:
	push byte 0
	push byte %1
	jmp isr_common_stub
%ENDMACRO

%IMACRO ISR_B 1
GLOBAL	_isr%1
ALIGN	4
_isr%1:
	push byte %1
	jmp isr_common_stub
%ENDMACRO

SECTION .kernel
isr_common_stub:
    pushad					; push all general purpose registers
    push ds					; push al the segments
    push es
    push fs
    push gs 
    mov ax, KERNEL_DATA_SEL	; set data segments for kernel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp				; push current stack pointer
    call _interrupt_dispatcher	; call out C interrupt_dispatcher() function
	add esp, 4				; clean up the previously pushed stack pointer
    pop gs					; pop al the segments
    pop fs
    pop es
    pop ds
    popad					; pop all general purpose registers
    add esp, 8				; clean up the two bytes we pushed on via the _isrXX routine
    iret					; iret back

_disable_int:
	iret

_disable_irqA:
	mov al, 0x20
	out 0x20, al
	iret

_disable_irqB:
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

_isr32:
    push byte 0					; we dont need these but push a dummy error code and int number so the PROCESS_STACK sructure fits properly
    push byte 32
    pushad						; push all general purpose registers
    push ds						; push al the segments
    push es
    push fs
    push gs  
    mov ax, KERNEL_DATA_SEL		; set data segments for kernel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax  
	mov [_current_esp], esp		; save the current esp
    call _scheduler_handler		; call out C scheduler_handler() function
    mov eax, [_current_cr3]		; switch over to the tasks address space
    mov cr3, eax
	mov esp, [_current_esp]		; restore esp (possibly a new one)
	mov al, 0x20
	out 0x20, al
    pop gs						; pop al the segments
    pop fs
    pop es
    pop ds  
    popad						; pop all general purpose registers
    add esp, 8					; clear off the two bytes we pushed at the begining
    iret						; iret back

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

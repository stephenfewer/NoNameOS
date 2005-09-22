BITS 32

KERNEL_VIRTUAL_ADDRESS	equ 0xC0000000

MULTIBOOT_PAGE_ALIGN	equ 1<<0
MULTIBOOT_MEMORY_INFO	equ 1<<1
MULTIBOOT_AOUT_KLUDGE	equ 1<<16
MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

EXTERN text, kernel, setup, data, bss, _end, _main

EXTERN _isr_dispatcher

GLOBAL _setup
GLOBAL _start

GLOBAL _isr00
GLOBAL _isr01
GLOBAL _isr02
GLOBAL _isr03
GLOBAL _isr04
GLOBAL _isr05
GLOBAL _isr06
GLOBAL _isr07
GLOBAL _isr08
GLOBAL _isr09
GLOBAL _isr10
GLOBAL _isr11
GLOBAL _isr12
GLOBAL _isr13
GLOBAL _isr14
GLOBAL _isr15
GLOBAL _isr16
GLOBAL _isr17
GLOBAL _isr18
GLOBAL _isr19
GLOBAL _isr20
GLOBAL _isr21
GLOBAL _isr22
GLOBAL _isr23
GLOBAL _isr24
GLOBAL _isr25
GLOBAL _isr26
GLOBAL _isr27
GLOBAL _isr28
GLOBAL _isr29
GLOBAL _isr30
GLOBAL _isr31

GLOBAL _irq00
GLOBAL _irq01
GLOBAL _irq02
GLOBAL _irq03
GLOBAL _irq04
GLOBAL _irq05
GLOBAL _irq06
GLOBAL _irq07
GLOBAL _irq08
GLOBAL _irq09
GLOBAL _irq10
GLOBAL _irq11
GLOBAL _irq12
GLOBAL _irq13
GLOBAL _irq14
GLOBAL _irq15

SECTION .setup

_setup:
    jmp short loader 
ALIGN 4
boot:
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM
loader:
	lgdt[GDTR]
	mov ax, DATA_SEGMENT
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp CODE_SEGMENT:KERNEL_VIRTUAL_ADDRESS
GDTR:				
	DW GDTEND-GDT-1	; limit
	DD GDT			; address
GDT:
NULL_SEGMENT	EQU $-GDT 
	DD	0x00000000
	DD	0x00000000
LINEARCODE_SEGMENT	EQU $-GDT
	DW	0xFFFF
	DW	0x0000
	DB	0x00
	DB	0x9A
	DB	0xCF
	DB	0x00
LINEARDATA_SEGMENT	EQU $-GDT
	DW	0xFFFF
	DW	0x0000
	DB	0x00
	DB	0x92
	DB	0xCF
	DB	0x00
CODE_SEGMENT	EQU $-GDT 
	DW	0xFFFF          
	DW	0x1000              
	DB	0x10              
	DB	0x9A             
	DB	0xCF            
	DB	0x40              
DATA_SEGMENT	EQU $-GDT  
	DW	0xFFFF           
	DW	0x1000              
	DB	0x10              
	DB	0x92             
	DB	0xCF             
	DB	0x40               
GDTEND:

SECTION .kernel

_start:
    mov esp, stack
    push dword 0
    popf

    xor eax, eax
    mov edi, bss
    mov ecx, _end
    sub ecx, edi
    shr ecx, 2
    rep stosd

	; fixup the GRUB MULTIBOOT_INFO pointer
	add ebx, KERNEL_VIRTUAL_ADDRESS
	push ebx

    call _main
    hlt
	
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    mov eax, _isr_dispatcher
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

;  Divide By Zero
_isr00:
    cli
    push byte 0
    push byte 0
    jmp isr_common_stub

;  Debug
_isr01:
    cli
    push byte 0
    push byte 1
    jmp isr_common_stub

;  Non Maskable Interrupt
_isr02:
    cli
    push byte 0
    push byte 2
    jmp isr_common_stub

;  Int 3
_isr03:
    cli
    push byte 0
    push byte 3
    jmp isr_common_stub

;  INTO
_isr04:
    cli
    push byte 0
    push byte 4
    jmp isr_common_stub

;  Out of Bounds
_isr05:
    cli
    push byte 0
    push byte 5
    jmp isr_common_stub

;  Invalid Opcode
_isr06:
    cli
    push byte 0
    push byte 6
    jmp isr_common_stub

;  Co-processor Not Available
_isr07:
    cli
    push byte 0
    push byte 7
    jmp isr_common_stub

;  Double Fault
_isr08:
    cli
    push byte 8
    jmp isr_common_stub

;  Coprocessor Segment Overrun
_isr09:
    cli
    push byte 0
    push byte 9
    jmp isr_common_stub

; Bad TSS
_isr10:
    cli
    push byte 10
    jmp isr_common_stub

; Segment Not Present
_isr11:
    cli
    push byte 11
    jmp isr_common_stub

; Stack Fault
_isr12:
    cli
    push byte 12
    jmp isr_common_stub

; General Protection Fault
_isr13:
    cli
    push byte 13
    jmp isr_common_stub

; Page Fault
_isr14:
    cli
    push byte 14
    jmp isr_common_stub

; Reserved
_isr15:
    cli
    push byte 0
    push byte 15
    jmp isr_common_stub

; Floating Point
_isr16:
    cli
    push byte 0
    push byte 16
    jmp isr_common_stub

; Alignment Check
_isr17:
    cli
    push byte 0
    push byte 17
    jmp isr_common_stub

; Machine Check
_isr18:
    cli
    push byte 0
    push byte 18
    jmp isr_common_stub

; Reserved
_isr19:
    cli
    push byte 0
    push byte 19
    jmp isr_common_stub

; Reserved
_isr20:
    cli
    push byte 0
    push byte 20
    jmp isr_common_stub

; Reserved
_isr21:
    cli
    push byte 0
    push byte 21
    jmp isr_common_stub

; Reserved
_isr22:
    cli
    push byte 0
    push byte 22
    jmp isr_common_stub

; Reserved
_isr23:
    cli
    push byte 0
    push byte 23
    jmp isr_common_stub

; Reserved
_isr24:
    cli
    push byte 0
    push byte 24
    jmp isr_common_stub

; Reserved
_isr25:
    cli
    push byte 0
    push byte 25
    jmp isr_common_stub

; Reserved
_isr26:
    cli
    push byte 0
    push byte 26
    jmp isr_common_stub

; Reserved
_isr27:
    cli
    push byte 0
    push byte 27
    jmp isr_common_stub

; Reserved
_isr28:
    cli
    push byte 0
    push byte 28
    jmp isr_common_stub

; Reserved
_isr29:
    cli
    push byte 0
    push byte 29
    jmp isr_common_stub

; Reserved
_isr30:
    cli
    push byte 0
    push byte 30
    jmp isr_common_stub

; Reserved
_isr31:
    cli
    push byte 0
    push byte 31
    jmp isr_common_stub

_irq00:
    cli
    push byte 0
    push byte 32
    jmp isr_common_stub

_irq01:
    cli
    push byte 0
    push byte 33
    jmp isr_common_stub

_irq02:
    cli
    push byte 0
    push byte 34
    jmp isr_common_stub

_irq03:
    cli
    push byte 0
    push byte 35
    jmp isr_common_stub

_irq04:
    cli
    push byte 0
    push byte 36
    jmp isr_common_stub

_irq05:
    cli
    push byte 0
    push byte 37
    jmp isr_common_stub

_irq06:
    cli
    push byte 0
    push byte 38
    jmp isr_common_stub

_irq07:
    cli
    push byte 0
    push byte 39
    jmp isr_common_stub

_irq08:
    cli
    push byte 0
    push byte 40
    jmp isr_common_stub

_irq09:
    cli
    push byte 0
    push byte 41
    jmp isr_common_stub

_irq10:
    cli
    push byte 0
    push byte 42
    jmp isr_common_stub

_irq11:
    cli
    push byte 0
    push byte 43
    jmp isr_common_stub

_irq12:
    cli
    push byte 0
    push byte 44
    jmp isr_common_stub

_irq13:
    cli
    push byte 0
    push byte 45
    jmp isr_common_stub

_irq14:
    cli
    push byte 0
    push byte 46
    jmp isr_common_stub

_irq15:
    cli
    push byte 0
    push byte 47
    jmp isr_common_stub
    
SECTION	.bss
ALIGN	16
RESB	4096
stack:

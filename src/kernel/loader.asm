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

KERNEL_VMA				equ 0xC0001000
KERNEL_LMA				equ 0x00101000
KERNEL_CODE_SEL			equ	0x08

MULTIBOOT_PAGE_ALIGN	equ 1<<0
MULTIBOOT_MEMORY_INFO	equ 1<<1
MULTIBOOT_AOUT_KLUDGE	equ 1<<16
MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

PAGE_DIRCTORY			equ	0x9C000
PAGE_TABLE_1			equ	0x9D000
PAGE_TABLE_2			equ	0x9E000
PRIV					equ	3

EXTERN text, kernel, data, bss, end, kernel_main

GLOBAL setup, start

SECTION .setup
setup:
	push ebx					;// store the pointer to the Grub multi boot header for later
	mov eax, PAGE_TABLE_1		;// create a page table that identity maps the first 4MB of mem
	mov ebx, 0x00000000 | PRIV	;// starting address of physical memory
	call map
	mov eax, PAGE_TABLE_2		;// create a page table that will map the kernel into the 3GB mark
	mov ebx, 0x00100000 | PRIV
	call map
	mov dword [PAGE_DIRCTORY +   0*4], PAGE_TABLE_1 | PRIV ;// store the first page table into the page directory
	mov dword [PAGE_DIRCTORY + 768*4], PAGE_TABLE_2 | PRIV ;// store the second page table into the page directory entry 768 (3GB mark)
	mov dword [PAGE_DIRCTORY + 1023*4], PAGE_DIRCTORY | PRIV;// store the page dir as the last entry in itself (fractal mapping)
	mov eax, PAGE_DIRCTORY		;// enable paging
	mov cr3, eax
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	pop ebx						;// restore ebx with the pointer to the multi boot header
	jmp KERNEL_CODE_SEL:KERNEL_VMA			;// jump into the kenel at it virtual memory address
map:
	mov ecx, 1024				;// loop 1024 times (number of entry's in a page table)
lmap:
	mov dword [eax], ebx		;// move next entry (ebx) into the page table (eax)
	add eax, 4					;// move forward to next entry in table
	add ebx, 4096				;// move address forward by a page size
	loop lmap					;// go again untill ecx == 0
	ret							;// return to caller
ALIGN 4
boot:
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM
SECTION .kernel
start:
    mov esp, stack
    push dword 0
    popf
    xor eax, eax
    mov edi, bss
    mov ecx, end
    sub ecx, edi
    shr ecx, 2
    rep stosd
	push ebx
    call kernel_main
    hlt
SECTION	.bss
ALIGN	16
RESB	4096
stack:

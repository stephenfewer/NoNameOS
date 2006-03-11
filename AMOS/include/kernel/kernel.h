#ifndef _KERNEL_KERNEL_H_
#define _KERNEL_KERNEL_H_

#include <sys/types.h>
#include <kernel/pm/elf.h>
#include <kernel/pm/process.h>

#define AMOS_VERSION_MAJOR			0
#define AMOS_VERSION_MINOR			5
#define AMOS_VERSION_PATCH			0

#define AMOS_VERSION_STRING			"AMOS 0.5.0"

#define KERNEL_PID					0

#define KERNEL_CODE_VADDRESS		(void *)0xC0000000

#define KERNEL_VGA_VADDRESS			(void *)0xE0000000

#define KERNEL_VGA_PADDRESS			(void *)0x000B8000

#define KERNEL_HEAP_VADDRESS		(void *)0xD0000000

struct MULTIBOOT_INFO
{
    DWORD flags;
    DWORD mem_lower;
    DWORD mem_upper;
    BYTE  boot_device[4];
    DWORD cmdline;
    DWORD mods_count;
    DWORD mods_addr;
    
    struct ELF_SECTION_HDR_TABLE elf_sec;
    
    DWORD mmap_length;
    DWORD mmap_addr;
    
    DWORD drives_length;
    DWORD drives_addr;
    DWORD config_table;
    DWORD boot_loader_name;
    DWORD apm_table;
    DWORD vbe_control_info;
    DWORD vbe_mode_info;
    WORD  vbe_mode;
    WORD  vbe_interface_seg;
    WORD  vbe_interface_off;
    WORD  vbe_interface_len;
};

BYTE inportb( WORD );

void outportb( WORD, BYTE );

void kernel_printInfo( void );

void kernel_printf( char *, ... );

void kernel_panic( struct PROCESS_STACK *, char * );

#endif


#ifndef _KERNEL_MM_PAGING_H_
#define _KERNEL_MM_PAGING_H_

#include <sys/types.h>
#include <kernel/isr.h>

#define PAGE_ENTRYS		1024

#define PAGE_SIZE		SIZE_4KB

#define PAGE_ALIGN( address )	( ( (address) + PAGE_SIZE - 1 ) & ~( PAGE_SIZE - 1 ) )

#define V2P( address ) (DWORD)( address - 0xC0001000 + 0x00101000 )

// see page 3-26
#define SUPERVISOR	0x00
#define USER		0x01

#define READONLY	0x00
#define READWRITE	0x01

// see page 3-21
#define OFFSET_MASK			0x03FF
#define DIRECTORY_SHIFT		22
#define TABLE_SHIFT			12

#define GET_DIRECTORY_INDEX( linearAddress )( ( linearAddress >> DIRECTORY_SHIFT ) & OFFSET_MASK )

#define GET_TABLE_INDEX( linearAddress )( ( linearAddress >> TABLE_SHIFT ) & OFFSET_MASK )

// From Intel IA32 Architecture Software Developers Manual Vol. 3 (3-24)
struct PAGE_DIRECTORY_ENTRY
{
    unsigned int present:1;
    unsigned int readwrite:1;
    unsigned int user:1;
    unsigned int writethrough:1;
    unsigned int cachedisabled:1;
    unsigned int accessed:1;
    unsigned int reserved:1;
    unsigned int pagesize:1;
    unsigned int globalpage:1;
    unsigned int available:3;
    unsigned int address:20;
} PACKED;

struct PAGE_TABLE_ENTRY
{
    unsigned int present:1;
    unsigned int readwrite:1;
    unsigned int user:1;
    unsigned int writethrough:1;
    unsigned int cachedisabled:1;
    unsigned int accessed:1;
    unsigned int dirty:1;
    unsigned int attributeindex:1;
    unsigned int globalpage:1;
    unsigned int available:3;
    unsigned int address:20;
} PACKED;

struct PAGE_DIRECTORY
{
    struct PAGE_DIRECTORY_ENTRY entry[PAGE_ENTRYS];
};

struct PAGE_TABLE
{
    struct PAGE_TABLE_ENTRY entry[PAGE_ENTRYS];
};

struct PAGE_DIRECTORY_ENTRY * paging_getPageDirectoryEntry( DWORD );

void paging_clearDirectory();

struct PAGE_TABLE_ENTRY * paging_getPageTableEntry( DWORD );

void paging_setPageTableEntry( DWORD, DWORD, BOOL );

void paging_setDirectoryTableEntry( DWORD, DWORD );

void paging_init();

void paging_handler( struct REGISTERS * );

#endif


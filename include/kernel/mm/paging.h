#ifndef _KERNEL_MM_PAGING_H_
#define _KERNEL_MM_PAGING_H_

#include <sys/types.h>
#include <kernel/pm/process.h>

#define PAGE_ENTRYS		1024

#define PAGE_SIZE		SIZE_4KB

#define PAGE_ALIGN( address )	(void *)( ( ((DWORD)address) + PAGE_SIZE - 1 ) & ~( PAGE_SIZE - 1 ) )

#define V2P( address ) (void *)( (DWORD)address - 0xC0001000 + 0x00101000 )

// see page 3-26
#define KERNEL		0x00	// called SUPERVISOR in the manual
#define USER		0x01

#define RING0		0
#define RING1		1
#define RING2		2
#define RING3		3

#define READONLY	0x00
#define READWRITE	0x01

// see page 3-21
#define OFFSET_MASK	0x03FF

#define DIRECTORY_SHIFT_R( address )		( (DWORD)address >> 22 )
#define TABLE_SHIFT_R( address )			( (DWORD)address >> 12 )
#define TABLE_SHIFT_L( address )			( (DWORD)address << 12 )

#define GET_PDE(v)		(struct PAGE_DIRECTORY_ENTRY *)(0xFFFFF000 + DIRECTORY_SHIFT_R(v) * 4)
#define GET_PTE(v)		(struct PAGE_TABLE_ENTRY *)(0xFFC00000 + TABLE_SHIFT_R(v) * 4)

#define GET_DIRECTORY_INDEX( linearAddress )( ( DIRECTORY_SHIFT_R(linearAddress) ) & OFFSET_MASK )

#define GET_TABLE_INDEX( linearAddress )( ( TABLE_SHIFT_R( linearAddress ) ) & OFFSET_MASK )

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

void paging_setCurrentPageDir( struct PAGE_DIRECTORY * );

struct PROCESS_INFO;

int paging_createDirectory( struct PROCESS_INFO * );

void paging_destroyDirectory( struct PROCESS_INFO * );

void paging_mapKernel( struct PROCESS_INFO * );

void * paging_mapQuick( void * );

void paging_map( struct PROCESS_INFO *, void *, void *, BOOL );

int paging_init( void );

#endif


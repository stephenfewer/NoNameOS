#include <kernel/mm/paging.h>
#include <kernel/mm/physical.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/isr.h>

extern void start;
extern void end;

struct PAGE_DIRECTORY * paging_pageDir;

struct PAGE_DIRECTORY * paging_getCurrentPageDir()
{
	struct PAGE_DIRECTORY * page_dir;
	ASM( "movl %%cr3, %0" : "=r" (page_dir) );
	return page_dir;
}

void paging_setCurrentPageDir( struct PAGE_DIRECTORY * page_dir )
{
	ASM( "movl %%eax, %%cr3" :: "r" ( page_dir ) );
}

struct PAGE_DIRECTORY_ENTRY * paging_getPageDirectoryEntry( void * linearAddress )
{
	return &paging_pageDir->entry[ GET_DIRECTORY_INDEX(linearAddress) ];
}

void paging_clearDirectory()
{
	int i=0;
	
	memset( (BYTE *)paging_pageDir, 0x00, sizeof(struct PAGE_DIRECTORY) );
	
	for( i=0 ; i<PAGE_ENTRYS; i++ )
	{
		struct PAGE_DIRECTORY_ENTRY * pde = &paging_pageDir->entry[i];
		pde->present = FALSE;
		pde->readwrite = READWRITE;
		pde->user = SUPERVISOR;
	}
}

void paging_setPageDirectoryEntry( void * linearAddress, void * ptAddress )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( linearAddress );
	memset( (BYTE *)ptAddress, 0x00, sizeof(struct PAGE_TABLE) );
	
	pde->present = TRUE;
	pde->readwrite = READWRITE;
	pde->user = SUPERVISOR;
	pde->writethrough = 0;
	pde->cachedisabled = 0;
	pde->accessed = 0;
	pde->reserved = 0;
	pde->pagesize = 0;
	pde->globalpage = 0;
	pde->available = 0;
	pde->address = TABLE_SHIFT_R(ptAddress);
}

struct PAGE_TABLE_ENTRY * paging_getPageTableEntry( void * linearAddress )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( linearAddress );
	struct PAGE_TABLE * pt = (struct PAGE_TABLE *)( TABLE_SHIFT_L(pde->address) );
	if( pt == NULL )
	{
		int i;
		pt = (struct PAGE_TABLE *)physical_pageAlloc();
		paging_setPageDirectoryEntry( linearAddress, (void *)pt );
		for( i=0 ; i<PAGE_ENTRYS ; i++ )
			paging_setPageTableEntry( linearAddress+SIZE_4KB, 0L, FALSE );
	}
	return (struct PAGE_TABLE_ENTRY *)&pt->entry[ GET_TABLE_INDEX(linearAddress) ];
}

// maps a linear address to a physical address
void paging_setPageTableEntry( void * linearAddress, void * physicalAddress, BOOL present )
{
	struct PAGE_TABLE_ENTRY * pte = paging_getPageTableEntry( PAGE_ALIGN( linearAddress ) );

	pte->present = present;
	pte->readwrite = READWRITE;
	pte->user = SUPERVISOR;
	pte->writethrough = 0;
	pte->cachedisabled = 0;
	pte->accessed = 0;
	pte->dirty = 0;
	pte->attributeindex = 0;
	pte->globalpage = 0;
	pte->available = 0;
	pte->address = TABLE_SHIFT_R( PAGE_ALIGN( physicalAddress ) );
	
	// Flush the TLB... invlpg is more efficent to use here
	ASM( "movl %cr3, %eax" );
	ASM( "movl %eax, %cr3" );
}

// See page 5-43
void paging_pageFaultHandler( struct REGISTERS * reg )
{
	void * linearAddress;
	ASM( "movl %%cr2, %0" : "=r" (linearAddress) );
	kprintf( "General Protection Fault at CS:EIP %d:%x Address %x\n", reg->cs, reg->eip, linearAddress );
	// we must hang untill we can fix the gpf
	while(TRUE);
}

void paging_init()
{
	void * physicalAddress;
	void * linearAddress;

	paging_pageDir = (struct PAGE_DIRECTORY *)physical_pageAlloc();

	// clear out the page directory...
	paging_clearDirectory();
		
	// identity map bottom 4MB's
	for( physicalAddress=0L ; physicalAddress<(void *)(1024*PAGE_SIZE) ; physicalAddress+=PAGE_SIZE )
		paging_setPageTableEntry( physicalAddress, physicalAddress, TRUE );		

	// map the kernel's virtual address to its physical memory location
	linearAddress = (void *)&start;
	for( physicalAddress=V2P(&start) ; physicalAddress<V2P(&end)+physical_getBitmapSize() ; physicalAddress+=PAGE_SIZE )
	{
		paging_setPageTableEntry( linearAddress, physicalAddress, TRUE );
		linearAddress += PAGE_SIZE;		
	}

	// Enable Paging...
	paging_setCurrentPageDir( paging_pageDir );
	
	ASM( "movl %cr0, %eax" );
	ASM( "orl $0x80000000, %eax" );
	ASM( "movl %eax, %cr0" );
}

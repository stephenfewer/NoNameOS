/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    Contact: steve [AT] harmonysecurity [DOT] com
 *    Web:     http://amos.harmonysecurity.com/
 *    License: GNU General Public License (GPL)
 */

#include <kernel/mm/paging.h>
#include <kernel/mm/physical.h>
#include <kernel/mm/mm.h>
#include <kernel/kernel.h>
#include <kernel/interrupt.h>
#include <kernel/pm/process.h>
#include <kernel/lib/string.h>

extern void start;
extern void end;

extern struct PROCESS_INFO kernel_process;

struct PAGE_DIRECTORY * paging_getCurrentPageDir()
{
	struct PAGE_DIRECTORY * pd;
	ASM( "movl %%cr3, %0" : "=r" ( pd ) );
	return pd;
}

void paging_setCurrentPageDir( struct PAGE_DIRECTORY * pd )
{
	// set cr3 to the physical address of the page directory
	ASM( "movl %%eax, %%cr3" :: "r" ( pd ) );
}

struct PAGE_DIRECTORY_ENTRY * paging_getPageDirectoryEntry( struct PAGE_DIRECTORY * pd, void * linearAddress )
{
	return &pd->entry[ GET_DIRECTORY_INDEX(linearAddress) ];
}

void paging_clearDirectory( struct PROCESS_INFO * p )
{
	int i=0;
	struct PAGE_DIRECTORY * pd;
	
	memset( (void *)p->page_dir, 0x00, sizeof(struct PAGE_DIRECTORY) );
	
	pd = p->page_dir;
	
	for( i=0 ; i<PAGE_ENTRYS; i++ )
	{
		struct PAGE_DIRECTORY_ENTRY * pde = &pd->entry[i];
		pde->present = FALSE;
		pde->readwrite = READWRITE;
		pde->user = p->privilege;
	}
}

void paging_setPageDirectoryEntry( struct PROCESS_INFO * p, void * linearAddress, void * ptAddress, BOOL clear )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( p->page_dir, linearAddress );
	
	//this is causing some page faults, only seems to be noticeably if we disable the paging_pageFaultHandler
	//if( clear )
	//	memset( ptAddress, 0x00, sizeof(struct PAGE_TABLE) );
	
	pde->present = TRUE;
	pde->readwrite = READWRITE;
	pde->user = p->privilege;
	pde->writethrough = 0;
	pde->cachedisabled = 0;
	pde->accessed = 0;
	pde->reserved = 0;
	pde->pagesize = 0;
	pde->globalpage = 0;
	pde->available = 0;
	pde->address = TABLE_SHIFT_R(ptAddress);
}

struct PAGE_TABLE_ENTRY * paging_getPageTableEntry( struct PROCESS_INFO * p, void * linearAddress )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( p->page_dir, linearAddress );
	struct PAGE_TABLE * pt = (struct PAGE_TABLE *)( TABLE_SHIFT_L(pde->address) );
	if( pt == NULL )
	{
		int i;
		pt = (struct PAGE_TABLE *)physical_pageAlloc();
		paging_setPageDirectoryEntry( p, linearAddress, (void *)pt, TRUE );
		for( i=0 ; i<PAGE_ENTRYS ; i++ )
			paging_setPageTableEntry( p, linearAddress+SIZE_4KB, 0L, FALSE );
	}
	return (struct PAGE_TABLE_ENTRY *)&pt->entry[ GET_TABLE_INDEX(linearAddress) ];
}

// maps a linear address to a physical address
void paging_setPageTableEntry( struct PROCESS_INFO * p, void * linearAddress, void * physicalAddress, BOOL present )
{
	struct PAGE_TABLE_ENTRY * pte = paging_getPageTableEntry( p, PAGE_ALIGN( linearAddress ) );

	pte->present = present;
	pte->readwrite = READWRITE;
	pte->user = p->privilege;
	pte->writethrough = 0;
	pte->cachedisabled = 0;
	pte->accessed = 0;
	pte->dirty = 0;
	pte->attributeindex = 0;
	pte->globalpage = 0;
	pte->available = 0;
	pte->address = TABLE_SHIFT_R( PAGE_ALIGN( physicalAddress ) );
}

extern struct PROCESS_INFO * scheduler_processCurrent;

// See page 5-43
DWORD paging_pageFaultHandler( struct PROCESS_STACK * stack )
{
	void * linearAddress;
	//kernel_lock();
	
	ASM( "movl %%cr2, %0" : "=r" (linearAddress) );
	kernel_printf( "Page Fault at CS:EIP %d:%x Address %x\n", stack->cs, stack->eip, linearAddress );

	// we must hang untill we can fix the page fault
	//while(TRUE);

	process_kill( scheduler_processCurrent->id );
	
	//kernel_unlock();
	return TRUE;
}

int paging_createDirectory( struct PROCESS_INFO * p )
{
	// allocate some physical memory for the page directory
	p->page_dir = (struct PAGE_DIRECTORY *)physical_pageAlloc();
	if( p->page_dir == NULL )
		return FALSE;
	// clear out the page directory
	paging_clearDirectory( p );
	// return success
	return TRUE;
}

// not tested but will be used to destroy a processes page directory when it terminates
void paging_destroyDirectory( struct PAGE_DIRECTORY * pd )
{
	struct PAGE_DIRECTORY_ENTRY * pde;
	struct PAGE_TABLE * pt;
	struct PAGE_TABLE_ENTRY * pte;
	void * physicalAddress;
	int i, x;
	// free up all the page tables we created but we dont free bottom 4 megs or kernel (anything above 3GB)
	// we also free the physical memory the page table entrys map
	for( i=1 ; i<768; i++ )
	{
		pde = &pd->entry[i];
		pt = (struct PAGE_TABLE *)( TABLE_SHIFT_L(pde->address) );
		if( pt != NULL )
		{
			// loop through all entrys in the page table
			for( x=0 ; x<PAGE_ENTRYS; x++ )
			{
				pte = &pt->entry[x];
				physicalAddress = (void *)TABLE_SHIFT_L( pte->address );
				// free the memory mapped to the page table entry
				if( physicalAddress != NULL )
					physical_pageFree( physicalAddress );
			}
			// free the page table
			physical_pageFree( pt );
		}
	}
	// free the page directory itself
	physical_pageFree( pd );
}

// map the kernel's virtual address to its physical memory location into pd
void paging_mapKernel( struct PROCESS_INFO * p )
{
	struct PAGE_DIRECTORY_ENTRY * pde;
	// map in the bottom 4MB's ( which are identity mapped, see paging_init() )
	pde = paging_getPageDirectoryEntry( kernel_process.page_dir, NULL );
	paging_setPageDirectoryEntry( p, NULL, (void *)TABLE_SHIFT_L(pde->address), FALSE );
	// map in the kernel
	pde = paging_getPageDirectoryEntry( kernel_process.page_dir, KERNEL_CODE_VADDRESS );
	paging_setPageDirectoryEntry( p, KERNEL_CODE_VADDRESS, (void *)TABLE_SHIFT_L(pde->address), FALSE );	
	// map in the kernel's heap
	pde = paging_getPageDirectoryEntry( kernel_process.page_dir, KERNEL_HEAP_VADDRESS );
	paging_setPageDirectoryEntry( p, KERNEL_HEAP_VADDRESS, (void *)TABLE_SHIFT_L(pde->address), FALSE );	
}

void paging_enable( void )
{
	ASM( "movl %cr0, %eax" );
	ASM( "orl $0x80000000, %eax" );
	ASM( "movl %eax, %cr0" );	
}

void paging_init()
{
	void * physicalAddress;
	void * linearAddress;

	// create the kernels page directory
	if( !paging_createDirectory( &kernel_process ) )
		kernel_panic( NULL, "Failed to create the kernels page directory." );

	// identity map bottom 4MB's
	for( physicalAddress=0L ; physicalAddress<(void *)(1024*PAGE_SIZE) ; physicalAddress+=PAGE_SIZE )
		paging_setPageTableEntry( &kernel_process, physicalAddress, physicalAddress, TRUE );		

	// map in the kernel
	physicalAddress = V2P( &start );
	linearAddress = (void *)&start;
	
	for( ; physicalAddress<V2P(&end)+physical_getBitmapSize() ; physicalAddress+=PAGE_SIZE )
	{
		paging_setPageTableEntry( &kernel_process, linearAddress, physicalAddress, TRUE );
		linearAddress += PAGE_SIZE;		
	}
	
	// set the system to use the kernels page directory
	paging_setCurrentPageDir( kernel_process.page_dir );
	
	// install the page fault handler
	interrupt_enable( INT14, paging_pageFaultHandler );

	// enable paging on the system
	paging_enable();
}

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
#include <kernel/mm/dma.h>
#include <kernel/mm/mm.h>
#include <kernel/kernel.h>
#include <kernel/interrupt.h>
#include <kernel/pm/scheduler.h>
#include <kernel/pm/process.h>
#include <lib/string.h>

extern void start;
extern void end;

extern struct PROCESS_INFO kernel_process;

void paging_setCurrentPageDir( struct PAGE_DIRECTORY * pd )
{
	// set cr3 to the physical address of the page directory
	ASM( "movl %0, %%cr3" :: "r" ( pd ) );
}

struct PAGE_DIRECTORY_ENTRY * paging_getPageDirectoryEntry( struct PAGE_DIRECTORY * pd, void * linearAddress )
{
	return &pd->entry[ GET_DIRECTORY_INDEX(linearAddress) ];
}

void paging_clearDirectory( struct PROCESS_INFO * p )
{
	int i;
	
	memset( p->page_dir, 0x00, sizeof(struct PAGE_DIRECTORY) );
	
	for( i=0 ; i<PAGE_ENTRYS; i++ )
	{
		struct PAGE_DIRECTORY_ENTRY * pde = &p->page_dir->entry[i];
		pde->present   = FALSE;
		pde->readwrite = READONLY;
		pde->user      = p->privilege;
	}
}

void paging_setPageDirectoryEntry( struct PROCESS_INFO * p, void * linearAddress, void * ptAddress )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( p->page_dir, linearAddress );
	
	pde->present       = TRUE;
	pde->readwrite     = READWRITE;
	pde->user          = p->privilege;
	pde->writethrough  = 0;
	pde->cachedisabled = 0;
	pde->accessed      = 0;
	pde->reserved      = 0;
	pde->pagesize      = 0;
	pde->globalpage    = 0;
	pde->available     = 0;
	pde->address       = TABLE_SHIFT_R(ptAddress);
}

struct PAGE_TABLE_ENTRY * paging_getPageTableEntry( struct PROCESS_INFO * p, void * linearAddress )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( p->page_dir, linearAddress );
	struct PAGE_TABLE * pt = (struct PAGE_TABLE *)( TABLE_SHIFT_L(pde->address) );
	if( pt == NULL )
	{
		int i;
		pt = (struct PAGE_TABLE *)physical_pageAlloc();
		paging_setPageDirectoryEntry( p, linearAddress, (void *)pt );
		for( i=0 ; i<PAGE_ENTRYS ; i++ )
			paging_setPageTableEntry( p, linearAddress+(i*SIZE_4KB), NULL, FALSE );
	}
	return (struct PAGE_TABLE_ENTRY *)&pt->entry[ GET_TABLE_INDEX(linearAddress) ];
}

// maps a linear address to a physical address
void paging_setPageTableEntry( struct PROCESS_INFO * p, void * linearAddress, void * physicalAddress, BOOL present )
{
	struct PAGE_TABLE_ENTRY * pte = paging_getPageTableEntry( p, PAGE_ALIGN( linearAddress ) );

	pte->present        = present;
	pte->readwrite      = READWRITE;
	pte->user           = p->privilege;
	pte->writethrough   = 0;
	pte->cachedisabled  = 0;
	pte->accessed       = 0;
	pte->dirty          = 0;
	pte->attributeindex = 0;
	pte->globalpage     = 0;
	pte->available      = 0;
	pte->address        = TABLE_SHIFT_R( PAGE_ALIGN( physicalAddress ) );
}

// See page 5-43
struct PROCESS_INFO * paging_pageFaultHandler( struct PROCESS_INFO * process )
{
//struct PROCESS_INFO * dr0process;
	void * linearAddress;
	// retrieve the linear address of the page fault stored in CR2
	ASM( "movl %%cr2, %0" : "=r" (linearAddress) );
	
//ASM( "movl %%dr0, %%eax" :"=r" ( dr0process ): );
	
	kernel_printf( "Page Fault at CS:EIP %x:%x Address %x\n", process->kstack->cs, process->kstack->eip, linearAddress );
	
//kernel_printf( "dr0 %x process %x\n", dr0process, process );
	
	// if the kernel caused the page fault we must kernel panic
	if( process->id == KERNEL_PID )
		kernel_panic( process->kstack, "Kernel Page Fault." );
	// print out the stack
	process_printStack( process->kstack );
	// try to kill the offending process
	if( process_kill( process->id ) == SUCCESS )
		return scheduler_select( process->next );
	// if we failed to kill the process we dont need to perform a context switch
	return process;
}

int paging_createDirectory( struct PROCESS_INFO * p )
{
	// allocate some physical memory for the page directory
	p->page_dir = (struct PAGE_DIRECTORY *)physical_pageAlloc();
	if( p->page_dir == NULL )
		return FAIL;
	// clear out the page directory
	paging_clearDirectory( p );
	// identity map the physical address of the page directory
	paging_setPageTableEntry( p, p->page_dir, p->page_dir, TRUE );
	// return success
	return SUCCESS;
}

// used to destroy a processes address space when it terminates
void paging_destroyDirectory( struct PROCESS_INFO * p )
{
	struct PAGE_DIRECTORY_ENTRY * pde;
	struct PAGE_TABLE * pt;
	struct PAGE_TABLE_ENTRY * pte;
	void * physicalAddress;
	int i, x;
	// free up all the page tables we created but we dont free bottom 4 megs or kernel (anything above 3GB)
	// we also free the physical memory the page table entrys map to
	for( i=1 ; i<((DWORD)KERNEL_CODE_VADDRESS/(PAGE_SIZE*PAGE_ENTRYS)) ; i++ )
	{
		pde = &p->page_dir->entry[i];
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
	physical_pageFree( p->page_dir );
}

// map the kernel's virtual address to its physical memory location into pd
void paging_mapKernel( struct PROCESS_INFO * p )
{
	struct PAGE_DIRECTORY_ENTRY * pde;
	// map in the bottom 4MB's ( which are identity mapped, see paging_init() )
	pde = paging_getPageDirectoryEntry( kernel_process.page_dir, NULL );
	paging_setPageDirectoryEntry( p, NULL, (void *)TABLE_SHIFT_L(pde->address) );
	
	//pde = paging_getPageDirectoryEntry( kernel_process.page_dir, DMA_PAGE_VADDRESS );
	//paging_setPageDirectoryEntry( p, DMA_PAGE_VADDRESS, (void *)TABLE_SHIFT_L(pde->address) );
	
	// map in the kernel (which wont be > 4MB)
	pde = paging_getPageDirectoryEntry( kernel_process.page_dir, KERNEL_CODE_VADDRESS );
	paging_setPageDirectoryEntry( p, KERNEL_CODE_VADDRESS, (void *)TABLE_SHIFT_L(pde->address) );
	
	pde = paging_getPageDirectoryEntry( kernel_process.page_dir, KERNEL_VGA_VADDRESS );
	paging_setPageDirectoryEntry( p, KERNEL_VGA_VADDRESS, (void *)TABLE_SHIFT_L(pde->address) );
	
	// map in the kernel's heap, only maps the first 4MB, if heap grows bigger we will have problems
	pde = paging_getPageDirectoryEntry( kernel_process.page_dir, KERNEL_HEAP_VADDRESS );
	paging_setPageDirectoryEntry( p, KERNEL_HEAP_VADDRESS, (void *)TABLE_SHIFT_L(pde->address) );	
}

void paging_enable( void )
{
	ASM( "movl %cr0, %eax" );
	ASM( "orl $0x80000000, %eax" );
	ASM( "movl %eax, %cr0" );	
}

int paging_init( void )
{
	void * physicalAddress;
	void * linearAddress;

	// create the kernels page directory
	if( paging_createDirectory( &kernel_process ) == FAIL )
		kernel_panic( NULL, "Failed to create the kernels page directory." );

	paging_setPageTableEntry( &kernel_process, DMA_PAGE_VADDRESS, DMA_PAGE_VADDRESS, TRUE );
			
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

	paging_setPageTableEntry( &kernel_process, KERNEL_VGA_VADDRESS, KERNEL_VGA_PADDRESS, TRUE );

	// set the system to use the kernels page directory
	paging_setCurrentPageDir( kernel_process.page_dir );
	
	// install the page fault handler
	interrupt_enable( INT14, paging_pageFaultHandler, SUPERVISOR );

	// enable paging on the system
	paging_enable();
	
	return SUCCESS;
}

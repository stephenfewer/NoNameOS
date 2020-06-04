/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <kernel/mm/paging.h>
#include <kernel/mm/physical.h>
#include <kernel/mm/dma.h>
#include <kernel/mm/mm.h>
#include <kernel/kernel.h>
#include <kernel/interrupt.h>
#include <kernel/pm/scheduler.h>
#include <kernel/pm/process.h>
#include <kernel/pm/sync/mutex.h>
#include <lib/libc/string.h>

#include <kernel/debug.h>

extern unsigned char * start;
extern unsigned char * end;

extern struct PROCESS_INFO kernel_process;

struct MUTEX paging_lock;

void paging_setCurrentPageDir( struct PAGE_DIRECTORY * pd )
{
	// set cr3 to the physical address of the page directory
	ASM( "movl %0, %%cr3" :: "r" ( pd ) );
}

struct PAGE_DIRECTORY_ENTRY * paging_getPageDirectoryEntry( struct PAGE_DIRECTORY * pd, void * linearAddress )
{
	// convert the physical address of pd into a virtual address before reading/writing
	pd = (struct PAGE_DIRECTORY *)paging_mapQuick( pd );
	// return the correct page directory entry, (linear address).
	return &pd->entry[ GET_DIRECTORY_INDEX(linearAddress) ];
}

void paging_setPageDirectoryEntry( struct PROCESS_INFO * p, void * linearAddress, struct PAGE_TABLE * pt )
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
	pde->address       = TABLE_SHIFT_R(pt);
}

void paging_setPageTableEntry( struct PROCESS_INFO *, void *, void *, BOOL );

struct PAGE_TABLE_ENTRY * paging_getPageTableEntry( struct PROCESS_INFO * p, void * linearAddress )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( p->page_dir, linearAddress );
	struct PAGE_TABLE * pt = (struct PAGE_TABLE *)( TABLE_SHIFT_L(pde->address) );
	if( pt == NULL )
	{
		int index;
		pt = (struct PAGE_TABLE *)physical_pageAlloc();
		paging_setPageDirectoryEntry( p, linearAddress, pt );
		for( index=0 ; index<PAGE_ENTRYS ; index++ )
			paging_setPageTableEntry( p, linearAddress+(index*SIZE_4KB), NULL, FALSE );
	}
	// convert the physical address of pt into a virtual address before reading/writing
	pt = (struct PAGE_TABLE *)paging_mapQuick( pt );
	// return the entry which is now a virtual address in the current address space
	return (struct PAGE_TABLE_ENTRY *)&pt->entry[ GET_TABLE_INDEX(linearAddress) ];
}

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
	void * linearAddress;
	// retrieve the linear address of the page fault stored in CR2
	ASM( "movl %%cr2, %0" : "=r" (linearAddress) );
	kernel_printf( "Page Fault at CS:EIP %x:%x Address %x\n", process->kstack->cs, process->kstack->eip, linearAddress );
	// if the kernel caused the page fault we must kernel panic
	if( process->id == KERNEL_PID )
		kernel_panic( process->kstack, "Kernel Page Fault." );
	// print out the stack
	process_printStack( process->kstack );
	// try to kill the offending process
	if( process_kill( process->id ) == SUCCESS )
		return scheduler_select( NULL );
	// if we failed to kill the process we dont need to perform a context switch
	return process;
}

int paging_createDirectory( struct PROCESS_INFO * p )
{
	int index;
	struct PAGE_DIRECTORY * pd;
	// lock critical section
	mutex_lock( &paging_lock );
	// allocate some physical memory for the page directory
	p->page_dir = (struct PAGE_DIRECTORY *)physical_pageAlloc();
	if( p->page_dir == NULL )
	{
		mutex_unlock( &paging_lock );
		return FAIL;
	}
	// map the p->page_dir physical address into the current address space so we can read/write to it
	pd = (struct PAGE_DIRECTORY *)paging_mapQuick( p->page_dir );
	// clear out the page directory
	memset( pd, 0x00, sizeof(struct PAGE_DIRECTORY) );
	// set some default entrys in the page directory
	for( index=0 ; index<PAGE_ENTRYS; index++ )
	{
		// get the next entry
		struct PAGE_DIRECTORY_ENTRY * pde = &pd->entry[index];
		// set the privilege ala the process
		pde->user = p->privilege;
		// if its the last entry
		if( index == PAGE_ENTRYS-1 )
		{
			// store the page dir as the last entry in itself (fractal mapping)
			pde->present	= TRUE;
			pde->readwrite	= READWRITE;
			pde->address	= TABLE_SHIFT_R( p->page_dir );		
		}
		else
		{
			pde->present   = FALSE;
			pde->readwrite = READONLY;
		}
	}
	// unlock critical section
	mutex_unlock( &paging_lock );
	// return success
	return SUCCESS;
}

// used to destroy a processes address space when it terminates
void paging_destroyDirectory( struct PROCESS_INFO * p )
{
	struct PAGE_DIRECTORY * pd;
	struct PAGE_DIRECTORY_ENTRY * pde;
	struct PAGE_TABLE * pt, * vpt;
	struct PAGE_TABLE_ENTRY * pte;
	void * physicalAddress;
	int i, x;
	// lock critical section
	mutex_lock( &paging_lock );
	// free up all the page tables we created but we dont free bottom 4 megs or kernel (anything above 3GB)
	// we also free the physical memory the page table entrys map to
	for( i=1 ; i<((DWORD)KERNEL_CODE_VADDRESS/(PAGE_SIZE*PAGE_ENTRYS)) ; i++ )
	{
		pd = (struct PAGE_DIRECTORY *)paging_mapQuick( p->page_dir );
		pde = &pd->entry[i];
		pt = (struct PAGE_TABLE *)( TABLE_SHIFT_L(pde->address) );
		if( pt != NULL && pde->present )
		{
			vpt = (struct PAGE_TABLE *)paging_mapQuick( pt );
			// loop through all entrys in the page table
			for( x=0 ; x<PAGE_ENTRYS; x++ )
			{
				pte = &vpt->entry[x];
				physicalAddress = (void *)TABLE_SHIFT_L( pte->address );
				// free the memory mapped to the page table entry
				if( physicalAddress != NULL && pte->present )
					physical_pageFree( physicalAddress );
			}
			// free the page table
			physical_pageFree( pt );
		}
	}
	// free the page directory itself
	physical_pageFree( p->page_dir );
	// unlock critical section
	mutex_unlock( &paging_lock );
}

// map the kernel into a process's address space
void paging_mapKernel( struct PROCESS_INFO * p )
{
	struct PAGE_DIRECTORY_ENTRY * pde;
	// lock critical section
	mutex_lock( &paging_lock );
	
	// map in the kernels bottom 4MB's ( which are identity mapped, see paging_init() )
	pde = paging_getPageDirectoryEntry( kernel_process.page_dir, NULL );
	paging_setPageDirectoryEntry( p, NULL, (struct PAGE_TABLE *)TABLE_SHIFT_L(pde->address) );

	// map in the kernel (which wont be > 4MB)
	pde = paging_getPageDirectoryEntry( kernel_process.page_dir, KERNEL_CODE_VADDRESS );
	paging_setPageDirectoryEntry( p, KERNEL_CODE_VADDRESS, (struct PAGE_TABLE *)TABLE_SHIFT_L(pde->address) );

	// map in the kernel's heap, only maps the first 4MB, if heap grows bigger we will have problems
	pde = paging_getPageDirectoryEntry( kernel_process.page_dir, KERNEL_HEAP_VADDRESS );
	paging_setPageDirectoryEntry( p, KERNEL_HEAP_VADDRESS, (struct PAGE_TABLE *)TABLE_SHIFT_L(pde->address) );

	// unlock critical section
	mutex_unlock( &paging_lock );
}

// map a physical page into the current address space so we can read/write to it
void * paging_mapQuick( void * physicalAddress )
{
	// get the pte via a lookup possible because of the fractal mapping of the page dir
	(GET_PTE( KERNEL_QUICKMAP_VADDRESS ))->address = TABLE_SHIFT_R( PAGE_ALIGN( physicalAddress ) );
	// ivalidate the TLB entry for our quick map virtual address
	ASM("invlpg %0" :: "m" (*(DWORD *)KERNEL_QUICKMAP_VADDRESS) : "memory" );
	// return the virtual address we mapped the physical page to
	return KERNEL_QUICKMAP_VADDRESS;
}

// maps a linear address to a physical address in process p's address space
void paging_map( struct PROCESS_INFO * p, void * linearAddress, void * physicalAddress, BOOL present )
{
	mutex_lock( &paging_lock );
	
	paging_setPageTableEntry( p, linearAddress, physicalAddress, present );

	mutex_unlock( &paging_lock );
}

int paging_init( void )
{
	void * physicalAddress;
	void * linearAddress;
	// init the lock
	mutex_init( &paging_lock );

	// create the kernels page directory
	if( paging_createDirectory( &kernel_process ) == FAIL )
		kernel_panic( NULL, "Failed to create the kernels page directory." );

	// identity map bottom 4MB's
	for( physicalAddress=NULL ; physicalAddress<(void *)(SIZE_1KB*SIZE_1KB*4) ; physicalAddress+=PAGE_SIZE )
		paging_map( &kernel_process, physicalAddress, physicalAddress, TRUE );

	// map in the kernel
	physicalAddress = V2P( &start );
	linearAddress = (void *)&start;

	for( ; physicalAddress<V2P(&end)+physical_getBitmapSize() ; physicalAddress+=PAGE_SIZE )
	{
		paging_map( &kernel_process, linearAddress, physicalAddress, TRUE );
		linearAddress += PAGE_SIZE;		
	}

	// mark the quickmap page table entry as present|supervisor|readwrite but dont set a physical address
	paging_map( &kernel_process, KERNEL_QUICKMAP_VADDRESS, NULL, TRUE );

	// set the system to use the kernels page directory
	paging_setCurrentPageDir( kernel_process.page_dir );
	
	// install the page fault handler
	interrupt_enable( INT14, paging_pageFaultHandler, KERNEL );
	
	return SUCCESS;
}

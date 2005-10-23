#include <kernel/mm/paging.h>
#include <kernel/mm/physical.h>
#include <kernel/mm/mm.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/isr.h>
#include <kernel/tasking/task.h>

extern void start;
extern void end;

struct PAGE_DIRECTORY * paging_kernelPageDir;

struct PAGE_DIRECTORY * paging_getCurrentPageDir()
{
	struct PAGE_DIRECTORY * pd;
	ASM( "movl %%cr3, %0" : "=r" ( pd ) );
	return pd;
}

void paging_setCurrentPageDir( struct PAGE_DIRECTORY * pd )
{
	ASM( "movl %%eax, %%cr3" :: "r" ( pd ) );
}

struct PAGE_DIRECTORY_ENTRY * paging_getPageDirectoryEntry( struct PAGE_DIRECTORY * pd, void * linearAddress )
{
	return &pd->entry[ GET_DIRECTORY_INDEX(linearAddress) ];
}

void paging_clearDirectory( struct PAGE_DIRECTORY * pd )
{
	int i=0;
	
	mm_memset( (BYTE *)pd, 0x00, sizeof(struct PAGE_DIRECTORY) );
	
	for( i=0 ; i<PAGE_ENTRYS; i++ )
	{
		struct PAGE_DIRECTORY_ENTRY * pde = &pd->entry[i];
		pde->present = FALSE;
		pde->readwrite = READWRITE;
		pde->user = SUPERVISOR;
	}
}

void paging_setPageDirectoryEntry( struct PAGE_DIRECTORY * pd, void * linearAddress, void * ptAddress, BOOL clear )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( pd, linearAddress );
	
	if( clear )
		mm_memset( (BYTE *)ptAddress, 0x00, sizeof(struct PAGE_TABLE) );
	
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

struct PAGE_TABLE_ENTRY * paging_getPageTableEntry( struct PAGE_DIRECTORY * pd, void * linearAddress )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( pd, linearAddress );
	struct PAGE_TABLE * pt = (struct PAGE_TABLE *)( TABLE_SHIFT_L(pde->address) );
	if( pt == NULL )
	{
		int i;
		pt = (struct PAGE_TABLE *)physical_pageAlloc();
		paging_setPageDirectoryEntry( pd, linearAddress, (void *)pt, TRUE );
		for( i=0 ; i<PAGE_ENTRYS ; i++ )
			paging_setPageTableEntry( pd, linearAddress+SIZE_4KB, 0L, FALSE );
	}
	return (struct PAGE_TABLE_ENTRY *)&pt->entry[ GET_TABLE_INDEX(linearAddress) ];
}

// maps a linear address to a physical address
void paging_setPageTableEntry( struct PAGE_DIRECTORY * pd, void * linearAddress, void * physicalAddress, BOOL present )
{
	struct PAGE_TABLE_ENTRY * pte = paging_getPageTableEntry( pd, PAGE_ALIGN( linearAddress ) );

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
	//ASM( "movl %cr3, %eax" );
	//ASM( "movl %eax, %cr3" );
}

// See page 5-43
DWORD paging_pageFaultHandler( struct TASK_STACK * taskstack )
{
	void * linearAddress;
	ASM( "movl %%cr2, %0" : "=r" (linearAddress) );
	kprintf( "Page Fault at CS:EIP %d:%x Address %x\n", taskstack->cs, taskstack->eip, linearAddress );
	// we must hang untill we can fix the page fault
	while(TRUE);
	return (DWORD)NULL;
}

struct PAGE_DIRECTORY * paging_createDirectory()
{
	struct PAGE_DIRECTORY * pd;
	// allocate some physical memory for the page directory
	pd = (struct PAGE_DIRECTORY *)physical_pageAlloc();
	// clear out the page directory
	paging_clearDirectory( pd );
	// return the new page directory
	return pd;
}

// not tested but will be used to destroy a tasks page directory when it terminates
void paging_destroyDirectory( struct PAGE_DIRECTORY * pd )
{
	int i;
	void * physicalAddress;
	// free up all the page tables we created, TO-DO: DONT FREE SHARED MEM LIKE KERNEL!! > 3GB
	for( i=0 ; i<PAGE_ENTRYS; i++ )
	{
		struct PAGE_DIRECTORY_ENTRY * pde = &pd->entry[i];
		physicalAddress = (void *)TABLE_SHIFT_L( pde->address );
		if( physicalAddress != NULL )
			physical_pageFree( physicalAddress );
	}
	// free the page directory itself
	physical_pageFree( pd );
}

// map the kernel's virtual address to its physical memory location into pd
void paging_mapKernel( struct PAGE_DIRECTORY * pd )
{
	void * physicalAddress = V2P( &start );
	void * linearAddress = (void *)&start;
	
	for( ; physicalAddress<V2P(&end)+physical_getBitmapSize() ; physicalAddress+=PAGE_SIZE )
	{
		paging_setPageTableEntry( pd, linearAddress, physicalAddress, TRUE );
		linearAddress += PAGE_SIZE;		
	}
}

void paging_mapKernelHeap( struct PAGE_DIRECTORY * pd )
{
	struct PAGE_DIRECTORY_ENTRY * pde;
	
	pde = paging_getPageDirectoryEntry( paging_kernelPageDir, KERNEL_HEAP_VADDRESS );
	paging_setPageDirectoryEntry( pd, KERNEL_HEAP_VADDRESS, TABLE_SHIFT_L(pde->address), FALSE );	
}

void paging_init()
{
	void * physicalAddress;

	// create the kernels page directory
	paging_kernelPageDir = paging_createDirectory();

	// identity map bottom 4MB's
	for( physicalAddress=0L ; physicalAddress<(void *)(1024*PAGE_SIZE) ; physicalAddress+=PAGE_SIZE )
		paging_setPageTableEntry( paging_kernelPageDir, physicalAddress, physicalAddress, TRUE );		

	// map the kernel into the kernels page directory
	paging_mapKernel( paging_kernelPageDir );

	// enable paging
	paging_setCurrentPageDir( paging_kernelPageDir );
	
	ASM( "movl %cr0, %eax" );
	ASM( "orl $0x80000000, %eax" );
	ASM( "movl %eax, %cr0" );
}

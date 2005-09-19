#include <kernel/mm/paging.h>
#include <kernel/mm/physical.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/isr.h>

struct PAGE_DIRECTORY * pagedirectory;

struct PAGE_DIRECTORY_ENTRY * paging_getPageDirectoryEntry( DWORD linearAddress )
{
	return (struct PAGE_DIRECTORY_ENTRY *)&pagedirectory->entry[ GET_DIRECTORY_INDEX(linearAddress) ];
}

void paging_setPageDirectoryEntry( DWORD linearAddress, DWORD ptAddress )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( linearAddress );
	
	memset( (BYTE *)pde, 0x00, sizeof(struct PAGE_DIRECTORY_ENTRY) );
	
	memset( (BYTE *)ptAddress, 0x00, sizeof(struct PAGE_TABLE) );
	
	pde->address = ptAddress >> TABLE_SHIFT;
	pde->present = TRUE;
	pde->readwrite = READWRITE;
}

struct PAGE_TABLE_ENTRY * paging_getPageTableEntry( DWORD linearAddress )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( linearAddress );
	
	struct PAGE_TABLE * pt = (struct PAGE_TABLE *)(pde->address << TABLE_SHIFT);
	
	if( pt == NULL )
	{
		pt = (struct PAGE_TABLE *)physical_pageAlloc();
		
		paging_setPageDirectoryEntry( linearAddress, (DWORD)pt );
	}
	
	return (struct PAGE_TABLE_ENTRY *)&pt->entry[ GET_TABLE_INDEX(linearAddress) ];
}

// maps a linear address to a physical address
void paging_setPageTableEntry( DWORD linearAddress, DWORD physicalAddress )
{
	struct PAGE_TABLE_ENTRY * pte = paging_getPageTableEntry( linearAddress );
	
	memset( (BYTE *)pte, 0x00, sizeof(struct PAGE_TABLE_ENTRY) );
	
	pte->address = physicalAddress >> TABLE_SHIFT;
	pte->present = TRUE;
	pte->readwrite = READWRITE;	
}

void paging_handler( struct REGISTERS * reg )
{
	kprintf( "paging_handler() - General Protection Fault!\n");
}

void paging_init()
{
	DWORD physicalAddress;
	
	isr_setHandler( 14, paging_handler );

	pagedirectory = (struct PAGE_DIRECTORY *)physical_pageAlloc();
	paging_setPageTableEntry( (DWORD)pagedirectory, (DWORD)pagedirectory );
	
	// clear out the page directory...
	memset( (BYTE *)pagedirectory, 0x00, sizeof(struct PAGE_DIRECTORY) );
	
	// identity map bottom 4MB's
	for( physicalAddress=0L ; physicalAddress<(1024*SIZE_4KB) ; physicalAddress+=SIZE_4KB )
	{
		paging_setPageTableEntry( physicalAddress, physicalAddress );		
	}

	// set cr3 = page directory address...
	__asm__ __volatile__ ( "movl %%eax, %%cr3" : : "a" ( pagedirectory ) );

	// enable paging...
	__asm__ __volatile__ ( "movl %cr0, %eax" );
	__asm__ __volatile__ ( "orl $0x80000000, %eax" );
	__asm__ __volatile__ ( "movl %eax, %cr0" );

}


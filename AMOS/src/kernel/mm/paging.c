#include <kernel/mm/paging.h>
#include <kernel/mm/physical.h>
#include <kernel/kernel.h>
#include <kernel/console.h>

struct PAGE_DIRECTORY * pagedirectory;

struct PAGE_DIRECTORY_ENTRY * paging_getPageDirectoryEntry( DWORD linearAddress )
{
	return (struct PAGE_DIRECTORY_ENTRY *)&pagedirectory->entry[ GET_DIRECTORY_INDEX(linearAddress) ];
}

struct PAGE_TABLE_ENTRY * paging_getPageTableEntry( DWORD linearAddress )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( linearAddress );
	
	struct PAGE_TABLE * pt = (struct PAGE_TABLE *)(pde->address << TABLE_SHIFT);

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

void paging_setPageDirectoryEntry( DWORD linearAddress, DWORD ptAddress )
{
	struct PAGE_DIRECTORY_ENTRY * pde = paging_getPageDirectoryEntry( linearAddress );
	
	memset( (BYTE *)pde, 0x00, sizeof(struct PAGE_DIRECTORY_ENTRY) );
	
	memset( (BYTE *)ptAddress, 0x00, sizeof(struct PAGE_TABLE) );
	
	pde->address = ptAddress >> TABLE_SHIFT;
	pde->present = TRUE;
	pde->readwrite = READWRITE;
}

void paging_init( DWORD mem_upper )
{
	int i, x, tables;
	DWORD physicalAddress = 0x00;
	
	tables = ( ( mem_upper / SIZE_1KB ) + 1 ) / 4;

	pagedirectory = (struct PAGE_DIRECTORY *)physical_pageAlloc();
	
	// clear out the page directory...
	memset( (BYTE *)pagedirectory, 0x00, sizeof(struct PAGE_DIRECTORY) );
	
	// identity map all physical memory!
	for( i=0 ; i<tables ; i++ )
	{
		paging_setPageDirectoryEntry( physicalAddress, physical_pageAlloc() );
		
		for( x=0 ; x<PAGE_ENTRYS ; x++ )
		{
			paging_setPageTableEntry( physicalAddress, physicalAddress );		
			physicalAddress += SIZE_4KB;
		}
	}

	// set cr3 = page directory address...
	__asm__ __volatile__ ( "movl %%eax, %%cr3" : : "a" ( pagedirectory ) );

	// enable paging...
	__asm__ __volatile__ ( "movl %cr0, %eax" );
	__asm__ __volatile__ ( "orl $0x80000000, %eax" );
	__asm__ __volatile__ ( "movl %eax, %cr0" );

}


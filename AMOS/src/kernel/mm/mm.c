#include <kernel/mm/physical.h>
#include <kernel/mm/paging.h>
#include <kernel/console.h>

void mm_init( DWORD mem_upper )
{
	physical_init( mem_upper );

	paging_init( mem_upper );
}

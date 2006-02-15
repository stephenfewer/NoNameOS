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

#include <kernel/pm/sync/mutex.h>
#include <kernel/mm/mm.h>

struct MUTEX * mutex_create()
{
	struct MUTEX * m;
	// create the MUTEX
	m = (struct MUTEX *)mm_malloc( sizeof(struct MUTEX) );
	mutex_init( m );
	// return it to caller
	return m;
}

void mutex_init( struct MUTEX * m )
{
	// set the lock to zero
	m->lock = 0;	
}

void mutex_lock( struct MUTEX * m )
{
	volatile DWORD unlocked;
	// we loop untill the lock has been reset
	// we could yield the processor and let another process run while we wait here
	while( !unlocked )
	{
		ASM( "lock" );
		ASM( "bts $0x01, %1" : "=r" (unlocked) : "m" (m->lock) : "memory" );		
		ASM( "sbbl %0, %0" : "=r" (unlocked) :: "memory" );	
	}
}

void mutex_unlock( struct MUTEX * m )
{
	// reset the lock
	ASM( "movb $0x00, %0" : "=m" (m->lock) :: "memory" );	
}

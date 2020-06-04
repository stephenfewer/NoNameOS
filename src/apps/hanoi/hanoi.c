/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <lib/amos.h>
#include <lib/libc/stdio.h>

void hanoi( int x, int y, int n )
{
	if( n )
    {
    	hanoi( x, 6 - (x + y), n - 1 );
    	hanoi( 6 - (x + y), y, n - 1 );
    }
}

void entrypoint( void )
{
	hanoi( 1, 3, 64 );
	
	exit();
}
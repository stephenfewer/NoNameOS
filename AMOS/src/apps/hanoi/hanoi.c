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

#include <lib/amos.h>
#include <lib/libc/stdio.h>

void hanoi( int x, int y, int n );

int main( void )
{
	hanoi( 1, 3, 64 );
	
	exit();
	
	return SUCCESS;
}

void hanoi( int x, int y, int n )
{
	if( n )
    {
    	hanoi( x, 6 - (x + y), n - 1 );
    	hanoi( 6 - (x + y), y, n - 1 );
    }
}

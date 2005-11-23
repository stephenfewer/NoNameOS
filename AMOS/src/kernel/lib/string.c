/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    License: GNU General Public License (GPL)
 */

#include <kernel/lib/string.h>

int strlen( char * src )
{
	int i=0;
	while( src[i] )
		i++;
	return i;
}

int strcmp( char * src, char * dest )
{
    while( *src && *src == *dest )
    {
        src++;
        dest++;
    }
    
    return( *src - *dest );
}

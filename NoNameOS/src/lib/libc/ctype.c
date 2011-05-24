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

#include <lib/libc/ctype.h>

int toupper( int ch )
{
	if( (unsigned int)(ch - 'a') < 26u )
		ch += 'A' - 'a';
	return ch;
}

int tolower( int ch )
{
	if( (unsigned int)(ch - 'A') < 26u )
		ch += 'a' - 'A';
	return ch;
}

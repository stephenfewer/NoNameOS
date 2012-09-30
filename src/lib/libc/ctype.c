/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
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

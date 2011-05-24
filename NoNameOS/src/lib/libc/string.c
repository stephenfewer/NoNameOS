/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <lib/libc/string.h>

int strlen( char * srs )
{
	char * s;
	for( s =srs ; *s!='\0' ; ++s );
	return s-srs;
}

char * strrchr( char * src, int c )
{
	char * p = src + strlen(src);
	do
	{
		if( *p == (char)c )
			return (char *)p;
	} while( --p >= src );
	
	return NULL;
}

int strcmp( char * src, char * dest )
{
	register signed char res;

	while( TRUE )
	{
		if( (res = *src - *dest++) != 0 || !*src++ )
			break;
	}
	return res;
}

int strncmp( char * src, char * dest, int count )
{
	register signed char res = 0;

	while( count )
	{
		if( (res = *src - *dest++) != 0 || !*src++ )
			break;
		count--;
	}

	return res;
}

char * strcpy( char * dest, char * src )
{
	char *tmp = dest;

	while( (*dest++ = *src++) != '\0' );

	return tmp;
}

char * strncpy( char * dest, char * src, int count )
{
	char *tmp = dest;

	while( count && (*dest++ = *src++) != '\0' )
		count--;

	return tmp;
}

char * strstr( char *s1, char *s2 )
{
	int l1, l2;
	l2 = strlen( s2 );
	if( !l2 )
		return (char *)s1;
	l1 = strlen( s1 );
	while( l1 >= l2 )
	{
		l1--;
		if( !memcmp( s1, s2, l2 ) )
			return (char *)s1;
		s1++;
	}
	return NULL;
}

void * memset( void * dest, BYTE val, int count )
{
    register char * d = (char *)dest;
    while( count-- )
    	*d++ = val;
    return dest;
}

void memcpy( void * dest, void * src, int count )
{
	register char * d = (char *)dest;
	register char * s = (char *)src;
	while( count-- )
		*d++ = *s++;
}

int memcmp( void *cs, void *ct, int count )
{
	const unsigned char *su1, *su2;
	signed char res = 0;

	for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

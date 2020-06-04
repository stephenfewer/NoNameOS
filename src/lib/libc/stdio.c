/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <lib/libc/stdio.h>
#include <lib/libc/string.h>
#include <lib/amos.h>

#define CONSOLE_SETACTIVE		1
#define CONSOLE_SENDCHAR		2
#define CONSOLE_SETBREAK		3
#define CONSOLE_SETECHO			4

void printf_putuint( int h, int i )
{
    unsigned int n, d = 1000000000;
    char str[ 255 ];
    unsigned int dec_index = 0;
    
    while( ( i / d == 0 ) && ( d >= 10 ) )
		d /= 10;
	
    n = i;
    
    while( d >= 10 )
    {
		str[ dec_index++ ] = ( ( char ) ( ( int ) '0' + n / d ) );
		n = n % d;
		d /= 10;
    }
    
    str[ dec_index++ ] = ( char ) ( ( int ) '0' + n );
    str[ dec_index ] = 0;

	write( h, (unsigned char *)&str, strlen(str) );
}

void printf_putint( int h, int i )
{
	if( i >= 0 )
	{
		printf_putuint( h, i );
	} else {
		write( h, (BYTE*)&"-", 1 );
		printf_putuint( h, -i );
	}
}

void printf_puthex( int h, DWORD i )
{
	const unsigned char hex[ 16 ]  =	{ '0', '1', '2', '3', '4', '5', '6', '7',
                            			  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    unsigned int n, d = 0x10000000;

    while( ( i / d == 0 ) && ( d >= 0x10 ) )
		d /= 0x10;
    
    n = i;
    
    while( d >= 0xF )
    {
    	write( h, (BYTE*)&hex[n / d], 1 );
		n = n % d;
		d /= 0x10;
    }
    
    write( h, (BYTE*)&hex[n], 1 );
}

void print( int h, char * text, va_list args )
{
	int i=0;
	BYTE * string;

	// sanity check
	if( h < 0 )
		return;

	while( text[i] )
	{
		if( text[i] == '%' )
		{
			i++;

			switch( text[i] )
			{
				case 's':
					string = va_arg( args, BYTE * );
					write( h, string, strlen( (char *)string ) );
					break;
				case 'c':
					// To-Do: fix this! "warning: cast to pointer from integer of different size"
					write( h, (BYTE*)(va_arg( args, BYTE )), 1 );
					break;
				case 'd':
					printf_putint( h, va_arg( args, int ) );
					break;
				case 'i':
					printf_putint( h, va_arg( args, int ) );
					break;
				case 'u':
					printf_putuint( h, va_arg( args, unsigned int ) );
					break;
				case 'x':
					printf_puthex( h, va_arg( args, DWORD ) );
					break;
				default:
					write( h, (BYTE*)&text[i], 1 );
			}

		}
		else
		{
			write( h, (BYTE*)&text[i], 1 );
		}
		
		i++;
	}

}

void printf( char * text, ... )
{
	va_list args;
	// find the first argument
	va_start( args, text );
	// pass print the output handle the format text and the first argument
	print( CONSOLE, text, args );
}

char getch()
{
	char c = -1;
	read( CONSOLE, (BYTE *)&c, 1 );
	return c;
}

int get( char * buffer, int size )
{
	// we want to echo what the user types to screen
	if( control( CONSOLE, CONSOLE_SETECHO, TRUE ) == FAIL )
		return -2;
	// set the break charachter to new line
	if( control( CONSOLE, CONSOLE_SETBREAK, '\n' ) == FAIL )
		return -3;
	// read in size byte or untill we reach a new line
	size = read( CONSOLE, (BYTE *)buffer, size );
	// stop echoing charachters to screen
	if( control( CONSOLE, CONSOLE_SETECHO, FALSE ) == FAIL )
		return -4;
	// add an end of line char if we did not fail
	if( size != FAIL )
		buffer[size] = 0x00;
	// return the amount of bytes we read in
	return size;
}

void putchar( char c )
{
	write( CONSOLE, (BYTE *)&c, sizeof( char ) );
}

void puts( char * buffer )
{
	while( *buffer )
		putchar( *buffer++ );
}

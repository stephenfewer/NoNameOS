/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

// prints out to Bochs

#include <kernel/io/port.h>
#include <kernel/debug.h>

void debug_putuint( int );
void debug_putch( BYTE );

void debug_putint( int i )
{
	if( i >= 0 )
	{
		debug_putuint( i );
	} else {
		debug_putch( '-' );
		debug_putuint( -i );
	}
}

void debug_puthex( DWORD i )
{
	const unsigned char hex[ 16 ]  =	{ '0', '1', '2', '3', '4', '5', '6', '7',
                            			  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    unsigned int n, d = 0x10000000;

	debug_putch( '0' );
	debug_putch( 'x' );
	
    while( ( i / d == 0 ) && ( d >= 0x10 ) )
		d /= 0x10;
    
    n = i;
    
    while( d >= 0xF )
    {
		debug_putch( hex[n / d] );
		n = n % d;
		d /= 0x10;
    }
    
    debug_putch( hex[n] );
}

void debug_putuint( int i )
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

	//vfs_write( h, (unsigned char *)&str, strlen(str) );
	n = 0;					
	while( str[n] ) 
		debug_putch( str[n++] );
}

void debug_putch( BYTE c )
{
	port_outb( 0xE9, c );
}

void debug_printf( char * text, va_list args )
{
	char dbgstr[] = "[AMOS] ";
	int i, j;
	BYTE * string;

	i = 0;

	while( dbgstr[i] )
	{
		debug_putch( dbgstr[i] );
		i++;
	}
	
	i = 0;
	
	while( text[i] )
	{
		if( text[i] == '%' )
		{
			i++;

			switch( text[i] )
			{
				case 's':
					string = va_arg( args, BYTE * );
					j = 0;
					while( string[j] ) 
						debug_putch( string[j++] );
					break;
				case 'c':
					debug_putch( va_arg( args, BYTE ) );
					break;
				case 'd':
					debug_putint( va_arg( args, int ) );
					break;
				case 'i':
					debug_putint( va_arg( args, int ) );
					break;
				case 'u':
					debug_putuint( va_arg( args, unsigned int ) );
					break;
				case 'x':
					debug_puthex( va_arg( args, DWORD ) );
					break;
				default:
					debug_putch( text[i] );
			}

			i++;

		} else {
			debug_putch( text[i] );
			i++;
		}

	}
}

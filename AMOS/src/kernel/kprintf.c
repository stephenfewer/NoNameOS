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

#include <kernel/kprintf.h>
#include <kernel/io/io.h>
#include <kernel/lib/string.h>

struct DEVICE_HANDLE * kprintf_consoleHandle = NULL;

void kprintf_putuint( int i )
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

	io_write( kprintf_consoleHandle, (unsigned char *)&str, strlen(str) );
}

void kprintf_putint( int i )
{
	if( i >= 0 )
	{
		kprintf_putuint( i );
	} else {
		io_write( kprintf_consoleHandle, (BYTE*)&"-", 1 );
		kprintf_putuint( -i );
	}
}

void kprintf_puthex( DWORD i )
{
	const unsigned char hex[ 16 ]  =	{ '0', '1', '2', '3', '4', '5', '6', '7',
                            			  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    unsigned int n, d = 0x10000000;

	io_write( kprintf_consoleHandle, (BYTE*)&"0", 1 );
	io_write( kprintf_consoleHandle, (BYTE*)&"x", 1 );
	
    while( ( i / d == 0 ) && ( d >= 0x10 ) )
		d /= 0x10;
    
    n = i;
    
    while( d >= 0xF )
    {
    	io_write( kprintf_consoleHandle, (BYTE*)&hex[n / d], 1 );
		n = n % d;
		d /= 0x10;
    }
    
    io_write( kprintf_consoleHandle, (BYTE*)&hex[n], 1 );
}

void kprintf( char * text, ... )
{
	int i;
	BYTE * string;
	va_list args;

	va_start( args, text );

	i = 0;
	
	if( kprintf_consoleHandle == NULL )
		kprintf_consoleHandle = io_open( "/device/console" );
		
	while( text[i] )
	{
		if( text[i] == '%' )
		{
			i++;

			switch( text[i] )
			{
				case 's':
					string = va_arg( args, BYTE * );
					io_write( kprintf_consoleHandle, string, strlen( (char *)string ) );
					break;
				case 'c':
					io_write( kprintf_consoleHandle, (BYTE*)(va_arg( args, BYTE )), 1 );
					break;
				case 'd':
					kprintf_putint( va_arg( args, int ) );
					break;
				case 'i':
					kprintf_putint( va_arg( args, int ) );
					break;
				case 'u':
					kprintf_putuint( va_arg( args, unsigned int ) );
					break;
				case 'x':
					kprintf_puthex( va_arg( args, DWORD ) );
					break;
				default:
					io_write( kprintf_consoleHandle, (BYTE*)&text[i], 1 );
			}

		}
		else
		{
			io_write( kprintf_consoleHandle, (BYTE*)&text[i], 1 );
		}
		
		i++;
	}

	va_end( args );
}

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

#include <kernel/lib/printf.h>
#include <kernel/io/io.h>
#include <kernel/lib/string.h>

void printf_putuint( struct IO_HANDLE * h, int i )
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

	io_write( h, (unsigned char *)&str, strlen(str) );
}

void printf_putint( struct IO_HANDLE * h, int i )
{
	if( i >= 0 )
	{
		printf_putuint( h, i );
	} else {
		io_write( h, (BYTE*)&"-", 1 );
		printf_putuint( h, -i );
	}
}

void printf_puthex( struct IO_HANDLE * h, DWORD i )
{
	const unsigned char hex[ 16 ]  =	{ '0', '1', '2', '3', '4', '5', '6', '7',
                            			  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    unsigned int n, d = 0x10000000;

	io_write( h, (BYTE*)&"0", 1 );
	io_write( h, (BYTE*)&"x", 1 );
	
    while( ( i / d == 0 ) && ( d >= 0x10 ) )
		d /= 0x10;
    
    n = i;
    
    while( d >= 0xF )
    {
    	io_write( h, (BYTE*)&hex[n / d], 1 );
		n = n % d;
		d /= 0x10;
    }
    
    io_write( h, (BYTE*)&hex[n], 1 );
}

void printf( struct IO_HANDLE * h, char * text, va_list args )
{
	int i=0;
	BYTE * string;

	while( text[i] )
	{
		if( text[i] == '%' )
		{
			i++;

			switch( text[i] )
			{
				case 's':
					string = va_arg( args, BYTE * );
					io_write( h, string, strlen( (char *)string ) );
					break;
				case 'c':
					// To-Do: fix this!
					io_write( h, (BYTE*)(va_arg( args, BYTE )), 1 );
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
					io_write( h, (BYTE*)&text[i], 1 );
			}

		}
		else
		{
			io_write( h, (BYTE*)&text[i], 1 );
		}
		
		i++;
	}

}

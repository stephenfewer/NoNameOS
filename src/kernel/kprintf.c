/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <kernel/kprintf.h>
#include <kernel/fs/vfs.h>
#include <kernel/io/dev/console.h>
#include <lib/libc/string.h>

void kprintf_putuint( struct VFS_HANDLE * h, int i )
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

	vfs_write( h, (unsigned char *)&str, strlen(str) );
}

void kprintf_putint( struct VFS_HANDLE * h, int i )
{
	if( i >= 0 )
	{
		kprintf_putuint( h, i );
	} else {
		vfs_write( h, (BYTE*)&"-", 1 );
		kprintf_putuint( h, -i );
	}
}

void kprintf_puthex( struct VFS_HANDLE * h, DWORD i )
{
	const unsigned char hex[ 16 ]  =	{ '0', '1', '2', '3', '4', '5', '6', '7',
                            			  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    unsigned int n, d = 0x10000000;

    while( ( i / d == 0 ) && ( d >= 0x10 ) )
		d /= 0x10;
    
    n = i;
    
    while( d >= 0xF )
    {
    	vfs_write( h, (BYTE*)&hex[n / d], 1 );
		n = n % d;
		d /= 0x10;
    }
    
    vfs_write( h, (BYTE*)&hex[n], 1 );
}

void kprintf( struct VFS_HANDLE * h, char * text, va_list args )
{
	int i=0, print_header=TRUE;
	BYTE * string;

	// sanity check
	if( h == NULL || text == NULL )
		return;

	if( strlen( text ) <= 1 )
		print_header = FALSE;

	if( print_header )
		vfs_write( h, (BYTE *)"[KERNEL] ", 9 );

	while( text[i] )
	{
		if( text[i] == '%' )
		{
			i++;

			switch( text[i] )
			{
				case 's':
					string = va_arg( args, BYTE * );
					vfs_write( h, string, strlen( (char *)string ) );
					break;
				case 'c':
					// To-Do: fix this! "warning: cast to pointer from integer of different size"
					vfs_write( h, (BYTE*)(va_arg( args, BYTE )), 1 );
					break;
				case 'd':
					kprintf_putint( h, va_arg( args, int ) );
					break;
				case 'i':
					kprintf_putint( h, va_arg( args, int ) );
					break;
				case 'u':
					kprintf_putuint( h, va_arg( args, unsigned int ) );
					break;
				case 'x':
					kprintf_puthex( h, va_arg( args, DWORD ) );
					break;
				default:
					vfs_write( h, (BYTE*)&text[i], 1 );
			}

		}
		else
		{
			vfs_write( h, (BYTE*)&text[i], 1 );
		}
		
		i++;
	}

}

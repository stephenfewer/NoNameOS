#include <kernel/kernel.h>
#include <kernel/debug.h>

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
}

void debug_putuint( int i )
{
    unsigned int n, d = 1000000000;
    BYTE ustr[ 255 ];
    unsigned int dec_index = 0;
    
    while( ( i / d == 0 ) && ( d >= 10 ) )
		d /= 10;
	
    n = i;
    
    while( d >= 10 )
    {
		ustr[ dec_index++ ] = ( ( BYTE ) ( ( int ) '0' + n / d ) );
		n = n % d;
		d /= 10;
    }
    
    ustr[ dec_index++ ] = ( BYTE ) ( ( int ) '0' + n );
    ustr[ dec_index ] = 0;
    					
    n = 0;					
	while( ustr[n] ) 
		debug_putch( ustr[n++] );
}

void debug_putch( BYTE c )
{
	outportb( 0xE9, c );
}

void debug_kprintf( char * text, ... )
{
	char dbgstr[] = "[AMOS] ";
	int i, j;
	BYTE * string;
	va_list args;

	va_start( args, text );

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

	va_end( args );
}




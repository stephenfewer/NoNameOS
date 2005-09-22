#include <kernel/kernel.h>
#include <kernel/console.h>

BYTE * console_mem;

int console_x, console_y;

BYTE console_attrib;

void kprintf( char * text, ... )
{
	int i, j;
	BYTE * string;
	va_list args;

	va_start( args, text );

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
						console_putch( string[j++] );
					break;
				case 'c':
					console_putch( va_arg( args, BYTE ) );
					break;
				case 'd':
					console_putint( va_arg( args, int ) );
					break;
				case 'i':
					console_putint( va_arg( args, int ) );
					break;
				case 'u':
					console_putuint( va_arg( args, unsigned int ) );
					break;
				case 'x':
					console_puthex( va_arg( args, DWORD ) );
					break;
				default:
					console_putch( text[i] );
			}

			i++;

		} else {
			console_putch( text[i] );
			i++;
		}

	}

	va_end( args );
}

void console_putint( int i )
{
	if( i >= 0 )
	{
		console_putuint( i );
	} else {
		console_putch( '-' );
		console_putuint( -i );
	}
}

void console_puthex( DWORD i )
{
	const unsigned char hex[ 16 ]  =	{ '0', '1', '2', '3', '4', '5', '6', '7',
                            			  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    unsigned int n, d = 0x10000000;

	console_putch( '0' );
	console_putch( 'x' );
	
    while( ( i / d == 0 ) && ( d >= 0x10 ) )
		d /= 0x10;
    
    n = i;
    
    while( d >= 0xF )
    {
		console_putch( hex[n / d] );
		n = n % d;
		d /= 0x10;
    }
}

void console_putuint( int i )
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
		console_putch( ustr[n++] );
}

void console_putch( BYTE c )
{
	switch( c )
	{
		case '\n':
			console_setCursor( 0, ++console_y );
			break;
		case '\r':
			console_setCursor( 0, console_y );
			break;
		case '\t':
			console_putch( ' ' );
			console_putch( ' ' );
			console_putch( ' ' );
			console_putch( ' ' );
			break;
		case '\a':
			console_beep();
			break;
		case '\f':
			console_cls();
			console_setCursor( 0, 0 );
			break;
		default:
			console_putChar( console_x, console_y, c, console_attrib );
			console_setCursor( ++console_x, console_y );
			break;
	}


	if( console_x > CONSOLE_COLUMNS-1 )
		console_setCursor( 0, ++console_y );

	if( console_y > CONSOLE_ROWS-1 )
	{
		console_scrollup();
		console_setCursor( console_x, CONSOLE_ROWS-1 );
	}
}

void console_putChar( int x, int y, BYTE c, BYTE attrib )
{
    console_mem[ (x + (y*CONSOLE_COLUMNS)) * 2 ] = c;
    console_mem[ ((x + (y*CONSOLE_COLUMNS)) * 2) + 1 ] = attrib;
}

BYTE console_getChar( int x, int y )
{
    return console_mem[ (x + (y*CONSOLE_COLUMNS)) * 2 ];
}

void console_setAttrib( BYTE attrib )
{
    console_attrib = attrib;
}

BYTE console_getAttrib( int x, int y )
{
    return console_mem[ ((x + (y*CONSOLE_COLUMNS)) * 2) + 1 ];
}

void console_scrollup( void )
{
	int i, j;

    for( j=1 ; j<CONSOLE_ROWS-1; j++ )
    {
		for( i=0; i<CONSOLE_COLUMNS ; i++ )
			console_putChar( i, j, console_getChar( i, j+1 ), console_getAttrib( i, j+1 ) );
    }

    for( i=0 ; i<CONSOLE_COLUMNS ; i++ )
		console_putChar( i, CONSOLE_ROWS-1, ' ', console_attrib );
}

void console_setCursor( int x, int y )
{
    short index = (y * CONSOLE_COLUMNS) + x;
    
	console_x = x;
    console_y = y;

	outportb( 0x3D4, 14 );
    outportb( 0x3D5, index >> 8 );
    outportb( 0x3D4, 15 );
    outportb( 0x3D5, index );
}

void console_cls( void )
{
	int i, j;

    for( i=0 ; i<CONSOLE_COLUMNS ; i++ )
    {
		for( j=0 ; j<CONSOLE_ROWS ; j++ )
			console_putChar( i, j, ' ', console_attrib );
    }
}

void console_beep( void )
{

}

void console_init( void )
{
    console_mem = (BYTE *)VIDEOMEM_BASE;

    console_setAttrib( GREEN | RED_BG );

    console_cls();

    console_setCursor( 0, 0 );
}


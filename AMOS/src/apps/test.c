#include <lib/amos.h>

int realmain( void );

void main( void )
{
	realmain();
	
	exit();
}

int realmain( void )
{
	write( CONSOLE, "hello from test.bin\n", 20 );
	return 0;
}

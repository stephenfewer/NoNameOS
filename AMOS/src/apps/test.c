#include <lib/amos.h>
#include <lib/printf.h>

int realmain( void );

void main( void )
{
	realmain();
	
	exit();
}

int realmain( void )
{
	printf( "hello from test.bin\n" );
	return 0;
}

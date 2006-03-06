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
	char buffer[256];
	
	printf( "Test App\n" );
	
	while( TRUE )
	{
		if( get( &buffer, 256 ) == FAIL )
			break;
		printf( "TEST GOT: %s\n", buffer );
	}

	// General Protection Fault
	//ASM( "cli" );
	
	// Page Fault
	//memset( (void *)0xD0000000, 0x00, 4096 );
	
	// Divide By Zero
	//ASM( "xor %ebx, %ebx; div %ebx" );

	printf( "Test App Finished.\n" );

	return 0;
}

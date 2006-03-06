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
	printf( "Test App\n" );
	
	// General Protection Fault
	//ASM( "cli" );
	
	// Page Fault
	//memset( (void *)0xD0000000, 0x00, 4096 );
	
	// Divide By Zero
	//ASM( "xor %ebx, %ebx; div %ebx" );

	printf( "Test App Finished.\n" );

	return 0;
}

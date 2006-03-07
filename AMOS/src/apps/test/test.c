#include <lib/amos.h>
#include <lib/printf.h>
#include <lib/string.h>

int realmain( void );

int main( void )
{
	realmain();
	
	exit();
	
	return 0;
}

int atoi( const char * s )
{
	long int v=0;
	int sign=1;
	
	while( *s == ' '  ||  (unsigned int)(*s - 9) < 5u ) s++;
	
	switch( *s )
	{
		case '-': sign=-1;
		case '+': ++s;
	}
	while( (unsigned int) (*s - '0') < 10u )
	{
		v=v*10+*s-'0'; ++s;
	}
	return sign==-1?-v:v;
}

int realmain( void )
{
	char buffer[16];

	while( TRUE )
	{
		printf( "Test App, please enter your choice:\n" );
		printf( "\t1. General Protection Fault\n" );
		printf( "\t2. Page Fault\n" );
		printf( "\t3. Divide By Zero\n" );

		if( get( (char *)&buffer, 16 ) == FAIL )
			break;

		switch( atoi( buffer ) )
		{
			case 1: // General Protection Fault
				printf( "About to execute a privileged instruction...\n" );
				ASM( "cli" );
				break;
			case 2:	// Page Fault
				printf( "About to memset the kernel heap...\n" );
				memset( (void *)0xD0000000, 0x00, 4096 );
				break;
			case 3:	// Divide By Zero
				printf( "About to divide by zero...\n" );
				ASM( "xor %ebx, %ebx; div %ebx" );
				break;
			default:
				printf( "Not a valid choice.\n" );
				break;	
		}
	}

	printf( "Finished.\n" );

	return 0;
}

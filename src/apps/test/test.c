/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <lib/amos.h>
#include <lib/libc/stdio.h>
#include <lib/libc/string.h>
#include <lib/libc/stdlib.h>

int stackoverflow( void )
{
	int i;
	int buffer[1024];
	// infinite recursion!!!
	for(i=0;i<1024;i++)
		buffer[i] = stackoverflow();
	return 0;
}

void entrypoint( void )
{
	char buffer[16];

	while( TRUE )
	{
		printf( "Test App\n" );
		printf( "\t1. Exit Gracefully!\n" );
		printf( "\t2. General Protection Fault\n" );
		printf( "\t3. Page Fault\n" );
		printf( "\t4. Divide By Zero\n" );
		printf( "\t5. Stack Overflow\n" );
		printf( "\t6. Invalid Opcode\n" );
		printf( "\t7. Loop Forever\n" );
		printf( "Please enter your choice: " );

		if( get( (char *)&buffer, 16 ) == FAIL )
			break;

		switch( atoi( buffer ) )
		{
			case 1:	// Exit Gracefully
				printf( "About to exit gacefully...\n" );
				exit();
			case 2: // General Protection Fault
				printf( "About to execute a privileged instruction...\n" );
				ASM( "cli" );
				break;
			case 3:	// Page Fault
				printf( "About to memset the kernel heap...\n" );
				memset( (void *)0xD0000000, 0x00, 4096 );
				break;
			case 4:	// Divide By Zero
				printf( "About to divide by zero...\n" );
				ASM( "xor %ebx, %ebx; div %ebx" );
				break;
			case 5:	// Stack Overflow
				printf( "About to cause a stack overflow...\n" );
				stackoverflow();
				break;
			case 6: // Invalid Opcode
				printf( "About to execute an invalid instruction...\n" );
				// undocumented instructions to generate an invalid
				// opcode interrupt for testing purposes 
				ASM( "ud2" );
				break;
			case 7: // Loop Forever
				printf( "About to loop forever...\n" );
				while( TRUE )
				{
					int i=9999999;
					while( --i );
					printf( " loop" );	
				}
				break;
			default:
				printf( "Not a valid choice.\n" );
				break;	
		}
	}

	printf( "Finished.\n" );

	exit();
}

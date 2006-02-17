#include <sys/types.h>
#include <kernel/syscall.h>

int foo( char c );

int main( void )
{
	unsigned char * VidMemChar = (unsigned char *)0xB8000;
	int ret=SYSCALL_TEST, num=SYSCALL_TEST;
	
	foo( '9' );
	
	ASM( "int $0x90" : "=a" (ret) : "a" (num) );

	foo( '8' );
	
	while( 1 )
	{
		if( *VidMemChar == '1' )
			foo( '2' );
		else
			foo( '1' );
	}
	
	return 0;
}

int foo( char c )
{
	unsigned char * VidMemChar = (unsigned char *)0xB8000;
	*VidMemChar = c;
	return 0;
}

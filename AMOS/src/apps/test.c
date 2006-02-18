#include <lib/amos.h>

int realmain( void );

int main( void )
{
	return realmain();
}

int realmain( void )
{
	unsigned char * VidMemChar = (unsigned char *)0xB8000;
	int handle;
	
	handle = open( "/device/console2", READWRITE );
	if( handle != 0 )
	{
		write( handle, "hello from test\n", 16 );
		
		close( handle );
	}
	
	*VidMemChar = '1';
	while( TRUE )
	{
		if( *VidMemChar == '1' )
			*VidMemChar = '2';
		else
			*VidMemChar = '1';
	}
	
	return 0;
}

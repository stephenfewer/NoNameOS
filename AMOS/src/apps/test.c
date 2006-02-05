#include <sys/types.h>

void main( void )
{
	unsigned char * VidMemChar = (unsigned char *)0xB8000;

	*VidMemChar = '1';

	while( 1 )
	{
		if( *VidMemChar == '1' )
			*VidMemChar = '2';
		else
			*VidMemChar = '1';
	}
}

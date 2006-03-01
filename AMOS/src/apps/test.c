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
	int i;
	while(TRUE)
	{
		printf( "test " );
		
		for(i=0;i<99999999;i++);
	}
	return 0;
}

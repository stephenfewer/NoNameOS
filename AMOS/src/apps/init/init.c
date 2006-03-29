#include <lib/amos.h>
#include <lib/printf.h>

int realmain( int, char ** );

int main( void )
{
	realmain( 0, NULL );
	
	exit();
	
	return 0;
}

int realmain( int argc, char **argv )
{
	printf( "Welcome! - Press keys F1 to F4 to navigate virtual consoles\n\n" );
	
	// spawn a shell on each virtual console
	spawn( "/amos/shell.bin", "/device/console1" );
	spawn( "/amos/shell.bin", "/device/console2" );
	spawn( "/amos/shell.bin", "/device/console3" );
	spawn( "/amos/shell.bin", "/device/console4" );
	
	return SUCCESS;
}

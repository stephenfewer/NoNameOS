/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    Contact: steve [AT] harmonysecurity [DOT] com
 *    Web:     http://amos.harmonysecurity.com/
 *    License: GNU General Public License (GPL)
 */

#include <lib/amos.h>
#include <lib/libc/stdio.h>

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
	
	// we could parse configuration files...
	
	// we could load other drivers...
	
	// we could mount other filesystems...
	
	// we could start background processes...
	
	// but for now we spawn a shell on each virtual console :)
	spawn( "/amos/shell.bin", "/amos/device/console1" );
	spawn( "/amos/shell.bin", "/amos/device/console2" );
	spawn( "/amos/shell.bin", "/amos/device/console3" );
	spawn( "/amos/shell.bin", "/amos/device/console4" );
	
	return SUCCESS;
}

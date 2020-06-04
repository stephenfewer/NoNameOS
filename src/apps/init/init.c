/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <lib/amos.h>
#include <lib/libc/stdio.h>

void entrypoint( void )
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
	
	exit();
}
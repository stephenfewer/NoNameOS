#include <lib/amos.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <apps/shell/tinysh.h>

#define SHELL_TITLE		"Shell Version 1.0"

static volatile int shell_canquit = FALSE;

void tinysh_char_out( unsigned char c );
static void shell_quit( int argc, char **argv );
static void shell_create( int argc, char **argv );
static void shell_delete( int argc, char **argv );
static void shell_rename( int argc, char **argv );
static void shell_copy( int argc, char **argv );
static void shell_list( int argc, char **argv );
//static void shell_mount( int argc, char **argv );
//static void shell_unmount( int argc, char **argv );
static void shell_spawn( int argc, char **argv );
static void shell_kill( int argc, char **argv );
void shell_exit( void );
void shell_args( int argc, char **argv );
int realmain(  int argc, char **argv );

int main( void )
{
	realmain( 0, NULL );
	
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

void tinysh_char_out( unsigned char c )
{
	write( CONSOLE, &c, 1 );
}

static void shell_quit( int argc, char **argv )
{
	printf( "quit: quitting...\n" );
	shell_canquit = TRUE;
}

static void shell_clear( int argc, char **argv )
{
	printf( "\f" );
}

static void shell_create( int argc, char **argv )
{
	if( argc < 2 )
	{
		printf("create: You must enter a file or directory to create.\n");
		return;
	}
	if( create( argv[1] ) == FAIL )
		printf("create: Failed to create %s.\n", argv[1] );
}

static void shell_delete( int argc, char **argv )
{
	if( argc < 2 )
	{
		printf("delete: You must enter a file to delete.\n");
		return;
	}
	if( delete( argv[1] ) == FAIL )
		printf("delete: Failed to delete file %s.\n", argv[1] );
}

static void shell_rename( int argc, char **argv )
{
	if( argc < 3 )
	{
		printf("rename: You must enter both a source and destenation file to rename.\n");
		return;
	}
	if( rename( argv[1], argv[2] ) == FAIL )
		printf("rename: Failed to rename file %s to %s.\n", argv[1], argv[2] );
}

static void shell_copy( int argc, char **argv )
{
	if( argc < 3 )
	{
		printf("copy: You must enter both a source and destenation file to copy.\n");
		return;
	}
	if( copy( argv[1], argv[2] ) == FAIL )
		printf("copy: Failed to copy file %s to %s.\n", argv[1], argv[2] );
}

#define TOTAL_ENTRYS		32

static void shell_list( int argc, char **argv )
{
	int i;
	DIRLIST_ENTRY entry[TOTAL_ENTRYS];
	char * dir;
	if( argc < 2 )
		dir = (char *)&"/";
	else
		dir = argv[1];
	
	if( list( dir, (DIRLIST_ENTRY *)&entry, TOTAL_ENTRYS ) == SUCCESS )
	{
		//printf( "list: %s\n", dir );
		for( i=0 ; i<TOTAL_ENTRYS ; i++ )
		{
			if( entry[i].name[0] == '\0' )
				break;
			printf( "\t%s %s\t\t%d\n",( entry[i].attributes==DIRECTORY ? "(D)" : "(F)" ), entry[i].name, entry[i].size );	
		}
	}
	else
	{
		printf( "list: Failed to list contents of %s.\n", dir );
	}
}

static void shell_dump( int argc, char **argv )
{
	int handle, size, bytesread;
	char buffer[BUFFER_SIZE];
	
	if( argc < 2 )
	{
		printf("dump: You must enter a filename to dump.\n");
		return;
	}
	
	handle = open( argv[1], MODE_READ );
	if( handle == FAIL )
	{
		printf("dump: Failed to open file %s.\n", argv[1] );
		return;
	}
	
	size = seek( handle, 0, SEEK_END );
	if( size <= FAIL )
	{
		printf("dump: Failed to file seek to end.\n" );
		close( handle );
		return;
	}

	if( seek( handle, 0, SEEK_START ) == FAIL )
	{
		printf("dump: Failed to file seek to start.\n" );
		close( handle );
		return;		
	}
	
	while( size>0 )
	{
		memset( (void *)&buffer, 0x00, BUFFER_SIZE );
		bytesread = read( handle, (BYTE *)&buffer, BUFFER_SIZE );
		if( bytesread == FAIL )
			break;
		size -= bytesread;
		printf( "%s", buffer );
	}
	
	close( handle );
	
	printf( "\n" );
}
/*
static void shell_mount( int argc, char **argv )
{

}

static void shell_unmount( int argc, char **argv )
{

}
*/
static void shell_spawn( int argc, char **argv )
{
	int shellwait = TRUE;
	int pid, i;
	
	if( argc < 2 )
	{
		printf("spawn: You must enter an executable filename to spawn.\n");
		return;
	}

	for( i=0 ; i<argc ; i++ )
	{
		if( strcmp( argv[i], "-b" ) == 0 )
			shellwait = FALSE;
	}
	
	pid = spawn( argv[argc-1], NULL );
	if( pid == FAIL )
	{
		printf("spawn: Failed to spawn process: %s.\n", argv[argc-1] );
	}
	else
	{
		printf("spawn: Spawned process %d\n", pid );
		if( shellwait )
			wait( pid );
	}
}

static void shell_kill( int argc, char **argv )
{
	int pid = 0;

	if( argc < 2 )
	{
		printf("kill: you must enter a process id to kill.\n");
		return;
	}
	// unlucky for some
	pid = atoi( argv[1] );
	if( pid == 0 )
	{
		printf("kill: you must enter a valid process id.\n");
		return;
	}
	
	if( kill( pid ) == FAIL )
		printf("kill: Failed to kill process %d.\n", pid );
}

static void shell_write( int argc, char **argv )
{
	int handle, i, byteswrite;
	char buffer[BUFFER_SIZE];
	int append=FALSE;
	
	if( argc < 2 )
	{
		printf("write: You must enter a filename to write to.\n");
		return;
	}
	
	for( i=0 ; i<argc ; i++ )
	{
		if( strcmp( argv[i], "-a" ) == 0 )
			append = TRUE;
	}
	
	handle = open( argv[argc-1], MODE_READWRITE | MODE_CREATE );
	if( handle == FAIL )
	{
		printf("write: Failed to open file %s\n", argv[argc-1] );
		return;
	}

	if( append )
	{
		if( seek( handle, 0, SEEK_END ) <= FAIL )
		{
			printf("write: Failed to file seek to end.\n" );
			close( handle );
			return;
		}
	}
	
	for(i=0;i<BUFFER_SIZE;i++)
		buffer[i] = 'S';
		
	byteswrite = write( handle, (BYTE *)&buffer, BUFFER_SIZE );
	if( byteswrite == FAIL )
	{
		printf("write: Failed to write.\n" );
		close( handle );
		return;
	}

	close( handle );
}

static tinysh_cmd_t clearcmd   = { 0, "clear",  "clear the console", 0, shell_clear, 0, 0, 0 };
static tinysh_cmd_t quitcmd    = { 0, "quit",   "exit the shell", 0, shell_quit, 0, 0, 0 };
static tinysh_cmd_t createcmd  = { 0, "create", "create a file or directory", "[name]", shell_create, 0, 0, 0 };
static tinysh_cmd_t deletecmd  = { 0, "delete", "delete a file", "[file]", shell_delete, 0, 0, 0 };
static tinysh_cmd_t renamecmd  = { 0, "rename", "rename a file", "[source file] [destination file]", shell_rename, 0, 0, 0 };
static tinysh_cmd_t copycmd    = { 0, "copy",   "copy a file", "[source file] [destination file]", shell_copy, 0, 0, 0 };
static tinysh_cmd_t listcmd    = { 0, "list",   "list directory contents", "<directory>", shell_list, 0, 0, 0 };
static tinysh_cmd_t dumpcmd    = { 0, "dump",   "dump a files contents to standard output", "[file]", shell_dump, 0, 0, 0 };
//static tinysh_cmd_t mountcmd   = { 0, "mount",  "mount a volume",   "[device] [mountpoint] [file system]", shell_mount, 0, 0, 0 };
//static tinysh_cmd_t unmountcmd = { 0, "unmount", "unmount a volume", "[mountpoint]", shell_unmount, 0, 0, 0 };
static tinysh_cmd_t spawncmd   = { 0, "spawn",  "spawn a process (-b background process)",  "-b [executable]", shell_spawn, 0, 0, 0 };
static tinysh_cmd_t killcmd    = { 0, "kill",   "kill a process",   "[process id]", shell_kill, 0, 0, 0 };
static tinysh_cmd_t writecmd   = { 0, "write",  "write fat test (-a append file)", "-a [file]", shell_write, 0, 0, 0 };

void shell_exit( void )
{
	// perform any tidy up here
	exit();
}

void shell_args( int argc, char **argv )
{
	int i;

	for( i=0 ; i<argc ; i++ )
	{
		if( strcmp( argv[i], "-h" ) == 0 )
		{
			printf("Shell Usage:\n");
			shell_exit();
		} else if( strcmp( argv[i], "-v" ) == 0 )
		{
			printf("%s\n", SHELL_TITLE );
			shell_exit();
		}
	}
}

int realmain( int argc, char **argv )
{
	tinysh_add_command( &clearcmd );
	tinysh_add_command( &quitcmd );    
	tinysh_add_command( &createcmd );
	tinysh_add_command( &deletecmd );
	tinysh_add_command( &renamecmd );
	tinysh_add_command( &copycmd );
	tinysh_add_command( &listcmd );
	tinysh_add_command( &dumpcmd );
	//tinysh_add_command( &mountcmd );
	//tinysh_add_command( &unmountcmd );
	tinysh_add_command( &spawncmd );
	tinysh_add_command( &killcmd );
	tinysh_add_command( &writecmd );

	printf("%s\n", SHELL_TITLE );

	tinysh_set_prompt( "AMOS:>" );
	
	while( !shell_canquit )
		tinysh_char_in( getch() );

	return SUCCESS;
}

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

// DOS has NUL:, Amiga OS has NIL:, Unix has /dev/null, we have /device/bitbucket

#include <kernel/io/dev/bitbucket.h>
#include <kernel/io/io.h>
#include <kernel/mm/mm.h>

struct IO_HANDLE * bitbucket_open( struct IO_HANDLE * handle, char * filename )
{
	// return the handle
	return handle;
}

int bitbucket_close( struct IO_HANDLE * handle )
{
	// return success
	return SUCCESS;
}

int bitbucket_write( struct IO_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	// return the number of bytes we were asked to write
	return size;
}

int bitbucket_init( void )
{
    struct IO_CALLTABLE * calltable;
	// setup the calltable for this driver
	calltable = (struct IO_CALLTABLE *)mm_malloc( sizeof(struct IO_CALLTABLE) );
	calltable->open = bitbucket_open;
	calltable->close = bitbucket_close;
	calltable->read = NULL;
	calltable->write = bitbucket_write;
	calltable->seek = NULL;
	calltable->control = NULL;
	// add the bitbucket device
	io_add( "bitbucket", calltable, IO_CHAR );
	return SUCCESS;
}

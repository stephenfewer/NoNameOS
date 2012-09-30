/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

// DOS has NUL:, Amiga OS has NIL:, Unix has /dev/null, we have /amos/device/bitbucket

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
	calltable = (struct IO_CALLTABLE *)mm_kmalloc( sizeof(struct IO_CALLTABLE) );
	calltable->open = bitbucket_open;
	calltable->close = bitbucket_close;
	calltable->clone = NULL;
	calltable->read = NULL;
	calltable->write = bitbucket_write;
	calltable->seek = NULL;
	calltable->control = NULL;
	// add the bitbucket device
	io_add( "bitbucket", calltable, IO_CHAR );
	return SUCCESS;
}

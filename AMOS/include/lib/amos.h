#ifndef _LIB_AMOS_H_
#define _LIB_AMOS_H_

#include <sys/types.h>
#include <kernel/fs/vfs.h>
#include <kernel/pm/process.h>

#define CONSOLE						PROCESS_CONSOLEHANDLE

#define MODE_READWRITE				VFS_MODE_READWRITE

typedef struct VFS_DIRLIST_ENTRY	DIRLIST_ENTRY;

int open( char *, int );

int close( int );

int read( int, BYTE *, DWORD );

int write( int, BYTE *, DWORD );

int seek( int, DWORD, BYTE );

int control( int, DWORD, DWORD );

int create( char * );

int delete( char * );

int rename( char *, char * );

int copy( char *, char * );

struct DIRLIST_ENTRY * list( char * );

int mount( char *, char *, int );

int unmount( char * );

void * morecore( DWORD );

void exit( void );

int spawn( char *, char * );

int kill( int );

int sleep( void );

int wake( int );

#endif 

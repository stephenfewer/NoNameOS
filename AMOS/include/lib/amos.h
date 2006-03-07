#ifndef _LIB_AMOS_H_
#define _LIB_AMOS_H_

#include <sys/types.h>
#include <kernel/fs/vfs.h>
#include <kernel/pm/process.h>

#define DIRECTORY					VFS_DIRECTORY
#define DEVICE						VFS_DEVICE
#define FILE						VFS_FILE

#define SEEK_START					VFS_SEEK_START
#define SEEK_CURRENT				VFS_SEEK_CURRENT
#define SEEK_END					VFS_SEEK_END

#define MODE_READ					VFS_MODE_READ
#define MODE_WRITE					VFS_MODE_WRITE
#define MODE_READWRITE				VFS_MODE_READWRITE
#define MODE_CREATE					VFS_MODE_CREATE
#define MODE_TRUNCATE				VFS_MODE_TRUNCATE
#define MODE_APPEND					VFS_MODE_APPEND

#define CONSOLE						PROCESS_CONSOLEHANDLE

typedef struct VFS_DIRLIST_ENTRY	DIRLIST_ENTRY;

int open( char *, int );

int close( int );

int clone( int );

int read( int, BYTE *, DWORD );

int write( int, BYTE *, DWORD );

int seek( int, DWORD, BYTE );

int control( int, DWORD, DWORD );

int create( char * );

int delete( char * );

int rename( char *, char * );

int copy( char *, char * );

int list( char *, DIRLIST_ENTRY *, int );

int mount( char *, char *, int );

int unmount( char * );

void * morecore( DWORD );

void exit( void );

int spawn( char *, char * );

int kill( int );

int sleep( void );

int wake( int );

int wait( int );

#endif 

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

#include <kernel/syscall.h>
#include <kernel/mm/mm.h>
#include <kernel/fs/vfs.h>
#include <kernel/interrupt.h>
#include <kernel/kernel.h>

struct SYSCALL syscall_table[SYSCALL_MAXCALLS];

DWORD syscall_handler( struct PROCESS_STACK * stack )
{
	int ret = FAIL;
	int index = (int)stack->eax;
	
	// make sure our syscall index into the syscall table is in range
	if( index < SYSCALL_MININDEX || index > SYSCALL_MAXINDEX )
		return FALSE;
	// make sure the syscall function has been set
	if( syscall_table[ index ].function.function != NULL )
	{
		switch( syscall_table[ index ].parameters  )
		{
			case 0:
				ret = SYSTEM_CALL0( syscall_table[ index ].function );
				break;
			case 1:
				ret = SYSTEM_CALL1( syscall_table[ index ].function, stack );
				break;
			case 2:
				ret = SYSTEM_CALL2( syscall_table[ index ].function, stack );
				break;	
			case 3:
				ret = SYSTEM_CALL3( syscall_table[ index ].function, stack );
				break;
			default:
				break;
		}
	}
	// se return value
	stack->eax = (DWORD)ret;
	// return to caller
	return FALSE;
}

BOOL syscall_add( int index, void * function, int parameters )
{
	// make sure our syscall index into the syscall table is in range
	if( index < SYSCALL_MININDEX || index > SYSCALL_MAXINDEX )
		return FALSE;
	// set the function & its number of parameters
	syscall_table[ index ].function.function = function;
	syscall_table[ index ].parameters = parameters;
	return TRUE;
}

int syscall_init( void )
{
	int index;
	// clear the system call table
	for( index=0 ; index<SYSCALL_MAXCALLS ; index++ )
	{	
		syscall_table[ index ].function.function = NULL;
		syscall_table[ index ].parameters = 0;
	}
	// add in all our system calls... file operations
	syscall_add( SYSCALL_OPEN,     vfs_open,	  2 );
	syscall_add( SYSCALL_CLOSE,    vfs_close,	  1 );
	syscall_add( SYSCALL_READ,     vfs_read,      3 );
	syscall_add( SYSCALL_WRITE,    vfs_write,     3 );
	syscall_add( SYSCALL_SEEK,     vfs_seek,      3 );
	syscall_add( SYSCALL_CONTROL,  vfs_control,   3 );	
	// file system operations
	syscall_add( SYSCALL_MOUNT,    vfs_mount,     3 );
	syscall_add( SYSCALL_UNMOUNT,  vfs_unmount,   1 );
	syscall_add( SYSCALL_CREATE,   vfs_create,    1 );
	syscall_add( SYSCALL_DELETE,   vfs_delete,    1 );
	syscall_add( SYSCALL_RENAME,   vfs_rename,    2 );
	syscall_add( SYSCALL_COPY,     vfs_copy,      2 );
	syscall_add( SYSCALL_LIST,     vfs_list,      1 );
	// memory operations
	syscall_add( SYSCALL_MORECORE, mm_morecore,   2 );
	// process operations
	//syscall_add( SYSCALL_SPAWN,  process_spawn, 1 );
	//syscall_add( SYSCALL_KILL,   process_kill,  1 );
	//syscall_add( SYSCALL_SLEEP,  process_sleep, 0 );
	//syscall_add( SYSCALL_WAKE,   process_wake,  1 );
	//syscall_add( SYSCALL_WAIT,   process_wait,  1 );

	// enable the system call interrupt
	// we need to set the privilage to USER (DPL = RING3) so it may be accessed from user mode
	interrupt_enable( SYSCALL_INTERRUPT, syscall_handler, USER );
	return SUCCESS;
}

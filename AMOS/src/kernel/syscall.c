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

SYSCALL syscall_table[SYSCALL_MAXCALLS];

int syscall_test( struct PROCESS_STACK * stack )
{
	char * message = (char *)stack->ebx;
	if( message == NULL )
		return -1;
	kernel_printf("SYSCALL TEST!!!\n" );
	return 0;
}

DWORD syscall_handler( struct PROCESS_STACK * stack )
{
	int index = 0;//(int)stack->eax;
	// default return value is a fail
	//stack->eax = (DWORD)-1;
kernel_printf("syscall handler, number %d\n", index );
/*	// make sure our syscall index into the syscall table is in range
	if( index < SYSCALL_MININDEX || index > SYSCALL_MAXINDEX )
		return FALSE;
	// make sure the syscall function has been set
	if( syscall_table[ index ] != NULL )
		stack->eax = (DWORD)syscall_table[ index ]( stack );
*/
	// return to caller
	return FALSE;
}

BOOL syscall_add( int index, SYSCALL function )
{
	// make sure our syscall index into the syscall table is in range
	if( index < SYSCALL_MININDEX || index > SYSCALL_MAXINDEX )
		return FALSE;
	// set the function
	syscall_table[ index ] = function;
	return TRUE;
}

void syscall_init( void )
{
	int index;
	// clear the system call table
	for( index=0 ; index<SYSCALL_MAXCALLS ; index++ )
		syscall_table[ index ] = NULL;

	syscall_add( SYSCALL_TEST, syscall_test );

	// add in all our system calls... file operations
/*	syscall_add( SYSCALL_OPEN,     vfs_open,      2 );
	syscall_add( SYSCALL_CLOSE,    vfs_close,     1 );
	syscall_add( SYSCALL_READ,     vfs_read,      3 );
	syscall_add( SYSCALL_WRITE,    vfs_write,     3 );
	syscall_add( SYSCALL_SEEK,     vfs_seek,      3 );
	syscall_add( SYSCALL_CONTROL,  vfs_seek,      3 );	
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
*/
	// enable the system call interrupt
	// we need to set the privilage to USER (DPL = RING3) so it may be accessed from user mode
	interrupt_enable( SYSCALL_INTERRUPT, syscall_handler, USER );
}

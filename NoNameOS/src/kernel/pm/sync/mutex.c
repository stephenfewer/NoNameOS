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

#include <kernel/pm/sync/mutex.h>
#include <kernel/pm/process.h>
#include <kernel/pm/scheduler.h>
#include <kernel/mm/mm.h>
#include <kernel/kernel.h>

void mutex_init( struct MUTEX * m )
{
	m->foo = 0L;
}

/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
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

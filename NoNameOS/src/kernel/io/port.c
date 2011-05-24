/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <kernel/io/port.h>

inline void port_outb( WORD port, BYTE data )
{
	ASM( "outb %%al, %%dx" :: "d" (port), "a" (data) );
}

inline void port_outw( WORD port, WORD data )
{
	ASM( "outw %%ax, %%dx" :: "d" (port), "a" (data) );
}

inline void port_outd( WORD port, DWORD data )
{
	ASM( "outl %%eax, %%dx" :: "d" (port), "a" (data) );
}

inline BYTE port_inb( WORD port )
{
	BYTE data;
	ASM( "inb %%dx, %%al" : "=a" (data) : "d" (port) );
	return data;
}

inline WORD port_inw( WORD port )
{
	WORD data;
	ASM( "inw %%dx, %%ax" : "=a" (data) : "d" (port) );
	return data;
}

inline DWORD port_ind( WORD port )
{
	DWORD data;
	ASM( "inl %%dx, %%eax" : "=a" (data) : "d" (port) );
	return data;
}

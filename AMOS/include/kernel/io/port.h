#ifndef _KERNEL_IO_PORT_H_
#define _KERNEL_IO_PORT_H_

#include <sys/types.h>

void port_outb( WORD, BYTE );

void port_outw( WORD, WORD );

void port_outd( WORD, DWORD );

BYTE port_inb( WORD );

WORD port_inw( WORD );

DWORD port_ind( WORD );

#endif

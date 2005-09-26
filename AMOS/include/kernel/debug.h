#ifndef _KERNEL_DEBUG_H_
#define _KERNEL_DEBUG_H_

#include <sys/types.h>

void debug_putint( int );

void debug_puthex( DWORD );

void debug_putuint( int );

void debug_putch( BYTE );

void debug_kprintf( char *, ... );

#endif

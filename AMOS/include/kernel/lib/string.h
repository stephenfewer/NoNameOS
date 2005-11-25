#ifndef _KERNEL_LIB_STRING_H_
#define _KERNEL_LIB_STRING_H_

#include <sys/types.h>

int strlen( char * );

int strcmp( char *, char * );

void * memset( void *, BYTE, int );

void memcpy( void *, void *, int );

#endif

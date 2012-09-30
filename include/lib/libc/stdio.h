#ifndef _LIB_LIBC_STDIO_H_
#define _LIB_LIBC_STDIO_H_

#include <sys/types.h>

void printf( char *, ... );

char getch();

int get( char *, int );

void putchar( char );

void puts( char * );

#endif

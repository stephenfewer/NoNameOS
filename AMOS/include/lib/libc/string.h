#ifndef _LIB_LIBC_STRING_H_
#define _LIB_LIBC_STRING_H_

#include <sys/types.h>

int strlen( char * );

char * strrchr( char *, int );

int strcmp( char *, char * );

int strncmp( char *, char *, int );

char * strcpy( char *, char * );

char * strncpy( char *, char *, int );

char *strstr( char *, char * );

void * memset( void *, BYTE, int );

void memcpy( void *, void *, int );

int memcmp( void *, void *, int );

#endif


#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

#define TRUE				0x01
#define FALSE				0x00

#define SUCCESS				0
#define FAIL				-1

#define ASM					__asm__ __volatile__
#define PACKED				__attribute__( (packed) )

#define NULL				((void *)0x00000000)

typedef int					BOOL;
typedef unsigned char		BYTE;
typedef unsigned short		WORD;
typedef unsigned long		DWORD;

#define SIZE_BYTE			1
#define SIZE_WORD			2
#define SIZE_DWORD			4

typedef char *				va_list;

#define __va_rounded_size( TYPE )	( ( (sizeof(TYPE) + sizeof(int) - 1) / sizeof(int) ) * sizeof(int) )

#define va_start( AP, LASTARG )		( AP = ((char *) &(LASTARG) + __va_rounded_size(LASTARG)) )

#define va_arg( AP, TYPE )			( AP += __va_rounded_size(TYPE), *((TYPE *) (AP - __va_rounded_size(TYPE))) )

#endif


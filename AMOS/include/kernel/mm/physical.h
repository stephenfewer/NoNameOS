#ifndef _KERNEL_MM_PHYSICAL_H_
#define _KERNEL_MM_PHYSICAL_H_

#include <sys/types.h>
#include <kernel/multiboot.h>

#define SIZE_4KB		4096
#define SIZE_1KB		1024

#define PHYSICAL_HIGH_PADDRESS		(void *)0x01000000			// 16MB ...the ISA DMA memory address limit
#define PHYSICAL_LOW_PADDRESS		(void *)(NULL + SIZE_4KB)	// the second page of physical memory

#define BITMAP_BYTE_INDEX( address )( ((DWORD)address / SIZE_4KB) / 8  )

#define BITMAP_BIT_INDEX( address )( 8 - (((DWORD)address / SIZE_4KB) % 8 ) - 1 )

#define physical_pageAllocAddress(a)( physical_bitmap[ BITMAP_BYTE_INDEX(a) ] |= ( 1 << BITMAP_BIT_INDEX(a) ) )

int physical_getBitmapSize();

void * physical_pageAllocHigh();

void * physical_pageAllocLow();

void * physical_pageAlloc();

void physical_pageFree( void * );

int physical_init( struct MULTIBOOT_INFO * );

#endif

#ifndef _KERNEL_IO_DEV_FLOPPY_H_
#define _KERNEL_IO_DEV_FLOPPY_H_

#include <sys/types.h>

#define FLOPPY_READBLOCK	1
#define FLOPPY_WRITEBLOCK	2

#define FLOPPY_TIMEOUT		128
#define FLOPPY_RWTRIES		3
#define FLOPPY_DMA_CHANNEL	2

// base addresses
#define FLOPPY_PRIMARY		0x03F0
#define FLOPPY_SECONDARY	0x0370

// registers
#define FLOPPY_DOR			0x02	// digital output register
#define FLOPPY_MSR			0x04	// main status register, input reg
#define FLOPPY_DRS			0x04	// data rate selector, output reg
#define FLOPPY_DATA			0x05	// data register
#define FLOPPY_CCR			0x07	// config controll reg

// commands
#define FLOPPY_RECALIBRATE	0x07
#define FLOPPY_SIS			0x08	// sence interrupt status
#define FLOPPY_SEEK			0x0F
#define FLOPPY_WRITE		0xC5
#define FLOPPY_READ			0xE6

struct FLOPPY_GEOMETRY
{
	int sectors;	
	int heads;
	int cylinders;
	
	int blocksize;
};

struct DOR_BITS
{
	unsigned int drive:2;
	unsigned int reset:1;
	unsigned int dma:1;
	unsigned int mota:1;
	unsigned int motb:1;
	unsigned int motc:1;
	unsigned int motd:1;
} PACKED;

struct DOR
{
	union
	{
		struct DOR_BITS bits;
		BYTE data;
	};
};

struct MSR_BITS
{
	unsigned mota:1;
	unsigned motb:1;
	unsigned motc:1;
	unsigned motd:1;
	unsigned busy:1;
	unsigned nondma:1;
	unsigned dio:1;
	unsigned mrq:1;
} PACKED;

struct MSR
{
	union
	{
		struct MSR_BITS bits;
		BYTE data;
	};
};

struct ST0_BITS
{
	unsigned us0:1;
	unsigned us1:1;
	unsigned hd:1;
	unsigned nr:1;
	unsigned uc:1;
	unsigned se:1;
	unsigned int_code:2;
} PACKED;

struct ST0
{
	union
	{
		struct ST0_BITS bits;
		BYTE data;
	};
};

struct FLOPPY_DRIVE
{
	char * name;
	BYTE locked;
	WORD base;
	int current_block;
	BYTE current_cylinder;
	struct ST0 st0;
	struct FLOPPY_GEOMETRY * geometry;
};

int floppy_init( void );

#endif

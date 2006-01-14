#ifndef _KERNEL_MM_DMA_H_
#define _KERNEL_MM_DMA_H_

#include <sys/types.h>

struct MRC_BITS
{
	unsigned int channel:2;
	unsigned int setclear:1;
	unsigned int unused:5;
} PACKED;

// MRC_BITS->channel ...see MODE_BITS->channel, its the same

// MODE_BITS->setclear
#define DMA_CLEARMASK	0
#define DMA_SETMASK		1

struct MODE_BITS
{
	unsigned int channel:2;
	unsigned int transfertype:2;
	unsigned int autoinit:1;
	unsigned int addressincrement:1;
	unsigned int modeselect:2;
} PACKED;

struct MODE
{
	union
	{
		struct MODE_BITS bits;
		BYTE data;
	};
};

// MODE_BITS->modeselect
#define DMA_DEMAND		0
#define DMA_SINGLE		1
#define DMA_BLOCK		2
#define DMA_CASCADE		3

// MODE_BITS->addressincrement
#define DMA_INCREMENT	0
#define DMA_DECREMENT	1

// MODE_BITS->autoinit
#define DMA_SINGLECYCLE	0
#define DMA_AUTOINIT	1

// MODE_BITS->transfertype
#define DMA_VERIFY	0
#define DMA_WRITE	1
#define DMA_READ	2

// MODE_BITS->channel
#define DMA_CHANNEL0	0
#define DMA_CHANNEL1	1
#define DMA_CHANNEL2	2
#define DMA_CHANNEL3	3
#define DMA_CHANNEL4	0
#define DMA_CHANNEL5	1
#define DMA_CHANNEL6	2
#define DMA_CHANNEL7	3

#define DMA_BUFFSIZE		8192
#define DMA_HALFBUFFSIZE	4096

void dma_read( BYTE, void *, DWORD );

void dma_write( BYTE, void *, DWORD );

#endif

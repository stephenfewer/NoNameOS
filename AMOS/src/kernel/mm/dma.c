/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    Contact: steve [AT] harmonysecurity [DOT] com
 *    Web:     http://amos.harmonysecurity.com/
 *    License: GNU General Public License (GPL)
 */

#include <kernel/mm/dma.h>
#include <kernel/kernel.h>
#include <kernel/interrupt.h>

BYTE dma_maskreg[8]     = { 0x0A, 0x0A, 0x0A, 0x0A, 0xD4, 0xD4, 0xD4, 0xD4 };
BYTE dma_modereg[8]     = { 0x0B, 0x0B, 0x0B, 0x0B, 0xD6, 0xD6, 0xD6, 0xD6 };
BYTE dma_clearreg[8]    = { 0x0C, 0x0C, 0x0C, 0x0C, 0xD8, 0xD8, 0xD8, 0xD8 };
// the page in memory which we are accessing
BYTE dma_pageport[8]    = { 0x87, 0x83, 0x81, 0x82, 0x8F, 0x8B, 0x89, 0x8A };
// the address port for data address
BYTE dma_addressport[8] = { 0x00, 0x02, 0x04, 0x06, 0xC0, 0xC4, 0xC8, 0xCC };
// the count port for the amount of data
BYTE dma_countport[8]   = { 0x01, 0x03, 0x05, 0x07, 0xC2, 0xC6, 0xCA, 0xCE };

void dma_transfer( BYTE channel, void * address, DWORD length, struct MODE mode )
{
	BYTE page;
	DWORD offset;
	// calculate the page
	page = (((DWORD)address) >> 16);
	// and the offset
	offset = (((DWORD)address) & 0xFFFF);
	// ...
	length--;
	// disable interrupts
	interrupt_disableAll();
	// ...
    outportb( dma_maskreg[channel], 0x04 | channel );
    // clear the byte pointer flip flop
    outportb( dma_clearreg[channel], 0x00 );
    // set the transfer mode
    outportb( dma_modereg[channel], mode.data );
    // set the address offset
    outportb( dma_addressport[channel], (offset & 0x00FF) );
    outportb( dma_addressport[channel], ((offset & 0xFF00) >> 8) );
    // set the page
    outportb( dma_pageport[channel], page );
    // set the length
    outportb( dma_countport[channel], (length & 0x00FF) );
    outportb( dma_countport[channel], ((length & 0xFF00) >> 8) );
    // ...
    outportb( dma_maskreg[channel], channel );
	// enable interrupts
    interrupt_enableAll();    
}

void dma_read( BYTE channel, void * address, DWORD length )
{
	struct MODE mode;
	// clear all the bits
	mode.data = 0x00;
	
	mode.bits.modeselect = DMA_SINGLE;
	mode.bits.transfertype = DMA_READ;
	mode.bits.channel = channel;

	dma_transfer( channel, address, length, mode );
}

void dma_write( BYTE channel, void * address, DWORD length )
{
	struct MODE mode;
	// clear all the bits
	mode.data = 0x00;
	
	mode.bits.modeselect = DMA_SINGLE;
	mode.bits.transfertype = DMA_WRITE;
	mode.bits.channel = channel;

	dma_transfer( channel, address, length, mode );
}

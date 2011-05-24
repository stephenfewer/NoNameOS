/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <kernel/io/dev/floppy.h>
#include <kernel/io/port.h>
#include <kernel/kernel.h>
#include <kernel/mm/physical.h>
#include <kernel/mm/mm.h>
#include <kernel/io/io.h>
#include <kernel/mm/dma.h>
#include <kernel/interrupt.h>
#include <kernel/pm/process.h>
#include <lib/libc/string.h>

struct FLOPPY_DRIVE * floppy1 = NULL;
struct FLOPPY_DRIVE * floppy2 = NULL;

// we must use the volatile keyword here to force the compiler to
// check the physical location and not optimize or cache this value
static volatile BYTE floppy_donewait = FALSE;

struct FLOPPY_GEOMETRY floppy_geometrys[6] = {
	{  0, 0,  0,   0 },	// no drive
	{  9, 2, 40, 512 },	// 360KB 5.25"
	{ 15, 2, 80, 512 },	// 1.2MB 5.25"
	{  9, 2, 80, 512 },	// 720KB 3.5"
	{ 18, 2, 80, 512 },	// 1.44MB 3.5"
	{ 36, 2, 80, 512 }	// 2.88MB 3.5"
};

// Send_Byte Routine as outlined in Figure 8.1 of the Intel 82077AA spec
void floppy_sendbyte( struct FLOPPY_DRIVE * floppy, BYTE byte )
{
    struct MSR msr;
    //initialize timeout counter
    int timeout = FLOPPY_TIMEOUT;
    // loop untill we timeout
    while( timeout-- )
    {
    	// read MSR
		msr.data = port_inb( floppy->base + FLOPPY_MSR );
		// loop untill ready status and FIFO direction is inward
		if( msr.bits.mrq && !msr.bits.dio )
		{
			// write byte to FIFO
		    port_outb( floppy->base + FLOPPY_DATA, byte );
		    return;
		}
		// delay
		port_inb( 0x80 );
    }	
}

// Get_Byte Routine as outlined in Figure 8.2 of the Intel 82077AA spec
BYTE floppy_getbyte( struct FLOPPY_DRIVE * floppy )
{
    struct MSR msr;
    //initialize timeout counter
    int timeout = FLOPPY_TIMEOUT;
    // loop untill we timeout
    while( timeout-- )
    {
    	// read MSR
		msr.data = port_inb( floppy->base + FLOPPY_MSR );
		// loop untill ready status and FIFO direction is outward
		if( msr.bits.mrq && msr.bits.dio )
			// return data byte from FIFO
		    return port_inb( floppy->base + FLOPPY_DATA );
	    // delay
		port_inb( 0x80 );
    }
    return FAIL;	
}

int floppy_on( struct FLOPPY_DRIVE * floppy )
{
	struct DOR dor;

	dor.data = 0x00;

	dor.bits.dma = TRUE;
	dor.bits.drive = ( floppy->base == FLOPPY_PRIMARY ? 0 : 1 );
	dor.bits.reset = TRUE;
	
	if( floppy->base == FLOPPY_PRIMARY )
		dor.bits.mota = TRUE;
	else
		dor.bits.motb = TRUE;
	
	port_outb( floppy->base + FLOPPY_DOR, dor.data );
	
	return SUCCESS;
}

int floppy_off( struct FLOPPY_DRIVE * floppy )
{
	struct DOR dor;
	// clear it
	dor.data = 0x00;

	dor.bits.dma = TRUE;
	dor.bits.drive = ( floppy->base == FLOPPY_PRIMARY ? 0 : 1 );
	dor.bits.reset = TRUE;
	
	port_outb( floppy->base + FLOPPY_DOR, dor.data );
	
	return SUCCESS;
}

struct PROCESS_INFO * floppy_handler( struct PROCESS_INFO * process )
{
	// set the donewait flag to signal a floppy_wait() to finish
	floppy_donewait = TRUE;

	return process;
}

int floppy_wait( struct FLOPPY_DRIVE * floppy, BYTE sence )
{
	// wait for interrupt to set the donewait flag
    while( TRUE )
    {
    	if( floppy_donewait == TRUE )
    		break;
    	process_yield();
    }
    // reset the donewait flag
    floppy_donewait = FALSE;
    
    if( sence )
    {
	    // issue a sence interrupt status command
		floppy_sendbyte( floppy, FLOPPY_SIS );
	    // get the result
	    floppy->st0.data = floppy_getbyte( floppy );
	    // get the current cylinder
	    floppy->current_cylinder = floppy_getbyte( floppy );
    }
    return SUCCESS;
}

int floppy_recalibrate( struct FLOPPY_DRIVE * floppy )
{
	// turn on the floppy motor
    floppy_on( floppy );
    // recalibrate the drive
    floppy_sendbyte( floppy, FLOPPY_RECALIBRATE );
    // specify which drive
    floppy_sendbyte( floppy, ( floppy->base == FLOPPY_PRIMARY ? 0 : 1 ) );
    // wait
    floppy_wait( floppy, TRUE );
    // turn off the motor
    floppy_off( floppy );
    return SUCCESS;
}

int floppy_seekcylinder( struct FLOPPY_DRIVE * floppy, BYTE cylinder )
{
	// check if we actually need to perform this operation
	if( floppy->current_cylinder == cylinder )
		return SUCCESS;
	// issue a seek command
	floppy_sendbyte( floppy, FLOPPY_SEEK );
	// specify drive
	floppy_sendbyte( floppy, ( floppy->base == FLOPPY_PRIMARY ? 0 : 1 ) );
	// specify cylinder
	floppy_sendbyte( floppy, cylinder );
	// wait for the controller to send an interrupt back
	floppy_wait( floppy, TRUE );
	// test if the seek operation performed correctly
	if( floppy->current_cylinder != cylinder )
		return FAIL;
	return SUCCESS;
}

int floppy_reset( struct FLOPPY_DRIVE * floppy )
{
	struct DOR dor;
	dor.data = 0x00;
	// disable the controller and irq and dma
	port_outb( floppy->base + FLOPPY_DOR, dor.data );
	// enable the controller
	dor.bits.dma = TRUE;
	dor.bits.reset = TRUE;
	port_outb( floppy->base + FLOPPY_DOR, dor.data );
	// wait for the controller to send an interrupt back
	floppy_wait( floppy, TRUE );
	// write 0x00 to the config controll register
	port_outb( floppy->base + FLOPPY_CCR, 0x00 );
	// recalibrate the drive
	floppy_recalibrate( floppy );
	return SUCCESS;
}

void floppy_blockGeometry( struct FLOPPY_DRIVE * floppy, int block, struct FLOPPY_GEOMETRY * geometry )
{
	// locate the correct cylinder
	geometry->cylinders = block / ( floppy->geometry->sectors * floppy->geometry->heads );
	// locate the correct head
	geometry->heads = ( block % ( floppy->geometry->sectors * floppy->geometry->heads ) ) / floppy->geometry->sectors;
	// locate the correct sector
	geometry->sectors = block % floppy->geometry->sectors + 1;
}

// based on Figure 8.5 (read/write) of the Intel 82077AA spec
int floppy_rwBlock( struct FLOPPY_DRIVE * floppy, void * buffer, int mode )
{
	void * dma_address;
	int tries = FLOPPY_RWTRIES;
	struct FLOPPY_GEOMETRY blockGeometry;
	// alloc a low physical memory address for the DMA buffer
	dma_address = physical_pageAllocLow();
	if( dma_address == NULL )
		return FAIL;
	// retrieve the block geometry
	floppy_blockGeometry( floppy, floppy->current_block, &blockGeometry );
	// if we are issuing a write command we copy the data into the DMA buffer first
	if( mode == FLOPPY_WRITEBLOCK )
		mm_pmemcpyto( dma_address, buffer, floppy->geometry->blocksize );
	// we try this 3 times as laid out int the Intel documentation
    while( tries-- )
    {			
    	// turn on the floppy motor
    	floppy_on( floppy );
    	// seek to the correct location
    	if( floppy_seekcylinder( floppy, blockGeometry.cylinders ) == FAIL )
    	{
    		kernel_printf("[floppy_rwBlock] read seek failed, block %d\n", floppy->current_block );
    		// break out off loop and fail
			break;	
    	}
		if( mode == FLOPPY_READBLOCK )
		{
			// setup DMA for a floppy read
			dma_write( FLOPPY_DMA_CHANNEL, dma_address, floppy->geometry->blocksize );
			// issue a read command
			floppy_sendbyte( floppy, FLOPPY_READ );
		}
		else
		{
			// setup DMA for a floppy write
			dma_read( FLOPPY_DMA_CHANNEL, dma_address, floppy->geometry->blocksize );
			// issue a write command
			floppy_sendbyte( floppy, FLOPPY_WRITE );
		}
    	// followed by the data
    	floppy_sendbyte( floppy, (blockGeometry.heads << 2) | ( floppy->base == FLOPPY_PRIMARY ? 0 : 1 ) );
    	floppy_sendbyte( floppy, blockGeometry.cylinders );
		floppy_sendbyte( floppy, blockGeometry.heads );
		floppy_sendbyte( floppy, blockGeometry.sectors );
		floppy_sendbyte( floppy, 2 );
		floppy_sendbyte( floppy, floppy->geometry->sectors );
		floppy_sendbyte( floppy, 0x1B );
		floppy_sendbyte( floppy, 0xFF );
		// wait for the floppy drive to send back an interrupt
		if( floppy_wait( floppy, FALSE ) == FAIL )
		{
			kernel_printf("[floppy_rwBlock] read floppy_wait failed, block %d\n", floppy->current_block );
			// if this fails reset the drive
			floppy_reset( floppy );
			// break out off loop and fail
			break;
		}
		// read in the result phase of the read command
		// we dont need the bytes we just need to clear the FIFO queue
		floppy_getbyte( floppy ); // st0
		floppy_getbyte( floppy ); // st1
		floppy_getbyte( floppy ); // st2
		floppy_getbyte( floppy ); // cylinder
		floppy_getbyte( floppy ); // head
		floppy_getbyte( floppy ); // sector number
		floppy_getbyte( floppy ); // sector size
		// test if operation was successfull, st0 was set by the previous floppy_wait() call
		if( floppy->st0.bits.int_code == 0x00 )
		{
			// turn off the floppy motor
    		floppy_off( floppy );
			// copy back buffer if we were reading
			if( mode == FLOPPY_READBLOCK )
				mm_pmemcpyfrom( buffer, dma_address, floppy->geometry->blocksize );
			// free the DMA buffer
    		physical_pageFree( dma_address );
			// return sucess :)
			return SUCCESS;
		}
    	// recalibrate the drive
		floppy_recalibrate( floppy );
    }
    kernel_printf("[floppy_rwBlock] read failed 3 times, block %d\n", floppy->current_block );
	// turn off the floppy motor
    floppy_off( floppy );
	// free the DMA buffer
	physical_pageFree( dma_address );
	// we fail if we cant read in three tries
	return FAIL;
}

int floppy_rw( struct IO_HANDLE * handle, BYTE * buffer, DWORD size, int mode  )
{
	int bytes_rw=FAIL, blocks=0;
	struct FLOPPY_DRIVE * floppy = (struct FLOPPY_DRIVE *)handle->data_ptr;
	// we can only write block alligned buffers of data
	if( size < floppy->geometry->blocksize || size % floppy->geometry->blocksize != 0 )
		return FAIL;
	// calculate how many blocks we need to read or write
	blocks = size / floppy->geometry->blocksize;
	// while we still have blocks to read or write, loop...
	while( blocks-- > 0 )
	{
		// try to read or write the next block
		if( floppy_rwBlock( floppy, buffer, mode ) == FAIL )
			break;
		// update the buffer pointer
		buffer += floppy->geometry->blocksize;
		// update the total amount of bytes read or written
		bytes_rw += floppy->geometry->blocksize;
		// increment the current block for the next read or write
		floppy->current_block++;
	}
	// return the total bytes read or written
	return bytes_rw;
}

struct IO_HANDLE * floppy_open( struct IO_HANDLE * handle, char * filename )
{
	// see if were trying to open floppy1
	if( strcmp( filename, floppy1->name ) == 0 )
	{
		// if its allready locked we cant open it
		if( floppy1->locked == TRUE )
			return NULL;
		// lock the drive
		floppy1->locked = TRUE;
		// associate floppy1 with the handle
		handle->data_ptr = floppy1;	
		// reset the drive
		floppy_reset( floppy1 );
	}
	else if( strcmp( filename, floppy2->name ) == 0 )
	{
		if( floppy2->locked == TRUE )
			return NULL;
		handle->data_ptr = floppy2;
		floppy2->locked = TRUE;
		// reset the drive
		floppy_reset( floppy2 );
	}
	else
	{
		// cant open the device
		return NULL;
	}
	// return the floppy handle
	return handle;	
}

int floppy_close( struct IO_HANDLE * handle)
{
	struct FLOPPY_DRIVE * floppy = (struct FLOPPY_DRIVE *)handle->data_ptr;
	// turn off the drive if its on
	floppy_off( floppy );
	// set the current block to zero for future access
	floppy->current_block = 0;
	// unlock the drive
	floppy->locked = FALSE;
	return SUCCESS;	
}

int floppy_read( struct IO_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	return floppy_rw( handle, buffer, size, FLOPPY_READBLOCK );	
}

int floppy_write( struct IO_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	return floppy_rw( handle, buffer, size, FLOPPY_WRITEBLOCK );
}

int floppy_seek( struct IO_HANDLE * handle, DWORD offset, BYTE origin )
{
	struct FLOPPY_DRIVE * floppy = (struct FLOPPY_DRIVE *)handle->data_ptr;
	// calculate the current block from the offset
	if( origin == VFS_SEEK_START )
		floppy->current_block = ( offset / floppy->geometry->blocksize );
	else if( origin == VFS_SEEK_CURRENT )
		floppy->current_block += ( offset / floppy->geometry->blocksize );
	//else if( origin == VFS_SEEK_END )
	//	floppy->current_block = TOTAL_FLOPPY_BLOCKS - ( offset / floppy->geometry->blocksize );
	else
		return FAIL;
	// returnt the current offset to the caller
	return floppy->current_block * floppy->geometry->blocksize;	
}

int floppy_init( void )
{
	BYTE i, floppy_type;
	struct IO_CALLTABLE * calltable;
	// create and initilise the call table
	calltable = (struct IO_CALLTABLE *)mm_kmalloc( sizeof(struct IO_CALLTABLE) );
	calltable->open = floppy_open;
	calltable->close = floppy_close;
	calltable->clone = NULL;
	calltable->read = floppy_read;
	calltable->write = floppy_write;
    calltable->seek = floppy_seek;
    calltable->control = NULL;
    // ask the CMOS if we have any floppy drives
    // location 0x10 has the floppy info
	port_outb( 0x70, 0x10 );
	i = port_inb( 0x71 );
	// detect the first floppy drive
	floppy_type = i >> 4;
	if( floppy_type != 0 )
	{
		floppy1 = (struct FLOPPY_DRIVE *)mm_kmalloc( sizeof(struct FLOPPY_DRIVE) );
		// set the device name
		floppy1->name = "floppy1";
		// set the floppy command base address
		floppy1->base = FLOPPY_PRIMARY;
		// set the correct floppy geometry
		floppy1->geometry = &floppy_geometrys[floppy_type];
		// unlock the drive
		floppy1->locked = FALSE;
		// add it to the DFS via the IO sub system
		io_add( floppy1->name, calltable, IO_BLOCK );
	}
	// detect the second floppy drive
    floppy_type = i & 0x0F;
 	if( floppy_type != 0 )
	{
		floppy2 = (struct FLOPPY_DRIVE *)mm_kmalloc( sizeof(struct FLOPPY_DRIVE) );
		// set the device name
		floppy2->name = "floppy2";
		// set the floppy command base address
		floppy2->base = FLOPPY_SECONDARY;
		// set the correct floppy geometry
		floppy2->geometry = &floppy_geometrys[floppy_type];
		// unlock the drive
		floppy2->locked = FALSE;
		// add it to the DFS via the IO sub system
		io_add( floppy2->name, calltable, IO_BLOCK );
	}
    // setup the floppy handler
	interrupt_enable( IRQ6, floppy_handler, KERNEL );
	// return success
	return SUCCESS;
}

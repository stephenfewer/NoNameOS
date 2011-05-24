/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <kernel/io/pci.h>
#include <kernel/io/port.h>
#include <kernel/mm/mm.h>
#include <kernel/kernel.h>
#include <sys/types.h>

struct PCI_DEVICEINFO * pci_deviceHead = NULL;

int pci_read( int bus, int dev, int func, int reg, int size )
{
    WORD base;
	struct PCI_CONF conf;

    conf.data = 0;
    conf.bits.enable = TRUE;
    conf.bits.bus = bus;
    conf.bits.dev = dev;
    conf.bits.func = func;
    conf.bits.reg = reg & 0xFC;

    port_outd( 0xCF8, conf.data );

    base = 0xCFC + ( reg & 0x03 );

    switch( size )
    {
	    case SIZE_BYTE:
	    	return port_inb( base );
	    case SIZE_WORD:
	    	return port_inw( base );
	    case SIZE_DWORD:
	    	return port_ind( base );
    }
    return FAIL;
}

int pci_write( int bus, int dev, int func, int reg, DWORD v, int size )
{
    WORD base;
	struct PCI_CONF conf;

    conf.data = 0;
    conf.bits.enable = TRUE;
    conf.bits.bus = bus;
    conf.bits.dev = dev;
    conf.bits.func = func;
    conf.bits.reg = reg & 0xFC;

    port_outd( 0xCF8, conf.data );
    
    base = 0xCFC + (reg & 0x03);
    
    switch( size )
    {
	    case SIZE_BYTE:
	    	port_outb( base, (BYTE)v );
	    	break;
	    case SIZE_WORD:
	    	port_outw( base, (WORD)v );
	    	break;
	    case SIZE_DWORD:
	    	port_outd( base, v );
	    	break;
    }
    return SUCCESS;
}


struct PCI_DEVICEINFO * pci_probeDevice( int bus, int dev, int func )
{
    struct PCI_DEVICEINFO * info;
    DWORD data;
    int index;
    // read in the vendor id
    data = pci_read( bus, dev, func, PCI_VENDORID, SIZE_WORD );
    // if we dont have one we return fail
    if( data == 0xFFFF || data == VENDOR_UNKNOWN )
    	return NULL;
    // if we have a vendor then we have a device. create a device info structure and fill it in
    info = (struct PCI_DEVICEINFO *)mm_kmalloc( sizeof(struct PCI_DEVICEINFO) );
    // fill in the vendor id    
    info->vendor_id = data;
    // fill in the device id   
    info->device_id = pci_read( bus, dev, func, PCI_DEVICEID, SIZE_WORD );
	// linear search for the vendor and device name based on the id's we just read in
	for( index=0 ; pci_devices[index].vendor_id != VENDOR_UNKNOWN ; index++ )
	{
		// if we have a match we break out of the loop
		if( pci_devices[index].vendor_id == info->vendor_id && pci_devices[index].device_id == info->device_id )
			break;
	}
	// fill in the vendor/device name into our device info structure
    info->vendor_name = pci_devices[index].vendor_name;
    info->device_name = pci_devices[index].device_name;
    // fill in the header type
    info->header_type = pci_read( bus, dev, func, PCI_HEADERTYPE, SIZE_BYTE );
    // fill in the sub sys info
    info->subsys_vendor = pci_read( bus, dev, func, PCI_SUBSYSVENDOR, SIZE_WORD );
    info->subsys = pci_read( bus, dev, func, PCI_SUBSYS, SIZE_WORD );
	// fill in the device class
	info->sub_class = pci_read( bus, dev, func, PCI_SUBCLASS, SIZE_BYTE );
	info->base_class = pci_read( bus, dev, func, PCI_BASECLASS, SIZE_BYTE );
	// linear search for the base class name
	for( index=0 ; pci_baseclassnames[index].base_class != 0xFF ; index++ )
	{
		if( pci_baseclassnames[index].base_class == info->base_class )
			break;
	}
	info->class_name = pci_baseclassnames[index].name;
	// fill in an irq value if their is one
	data = pci_read( bus, dev, func, PCI_IRQ, SIZE_BYTE );
	info->irq = ( data == 0xFF ? 0 : data );
	// return our new info structure
    return info;
}

void pci_probeBus( int bus )
{
	int device, function;
    // loop through each device on the bus
    for( device=0 ; device<PCI_MAXDEVICE ; device++ )
    {
        // loop through the functions on each device
        for( function=0 ; function<PCI_MAXFUNCTION ; function++ )
        {
            // use pci_probeDevice() to retrieve a DEVICEINFO structure if a device is present
			struct PCI_DEVICEINFO * dev = pci_probeDevice( bus, device, function );
			// if a device is not present we just continue our search for the next one
            if( dev == NULL )
				continue;
			// add the newly detected device to our backward linked list
			dev->prev = pci_deviceHead;
			pci_deviceHead = dev;
			// if the highest bit in the header is not set we dont scan all 8 functions
			if( (dev->header_type & 0x80) == 0 && function == 0 )
				break;
        }
    }
}

int pci_init( void )
{
	struct PCI_DEVICEINFO * dev;
	// initially we probe the first bus
	pci_probeBus( 0 );
	// search through the linked list of pci devices
	for( dev=pci_deviceHead ; dev!=NULL ; dev=dev->prev )
		kernel_printf( "[PCI] %s %s (%s)\n", dev->vendor_name, dev->device_name, dev->class_name );
    // return success
	return SUCCESS;
}

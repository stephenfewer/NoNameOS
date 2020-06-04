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

struct PCI_DEVICENAME pci_devices[] = {
	{ CIRRUS, 0xB8, CIRRUS_NAME, "GD 5446"},
	{ REALTEK, 0x8029, REALTEK_NAME, "8029"},
	{ REALTEK, 0x8129, REALTEK_NAME, "8129"},
	{ REALTEK, 0x8129, REALTEK_NAME, "8139"},
	{ INTEL, 0x0007, INTEL_NAME, "82379AB"},
	{ INTEL, 0x0008, INTEL_NAME, "Extended Express System Support Controller"},
	{ INTEL, 0x0482, INTEL_NAME, "82375EB"},
	{ INTEL, 0x0483, INTEL_NAME, "82424ZX Saturn"},
	{ INTEL, 0x0484, INTEL_NAME, "82378IB"},
	{ INTEL, 0x0486, INTEL_NAME, "82430ZX Aries"},
	{ INTEL, 0x04a3, INTEL_NAME, "82434LX Mercury/Neptune"},
	{ INTEL, 0x0960, INTEL_NAME, "i960"},
	{ INTEL, 0x1221, INTEL_NAME, "82092AA PCMCIA Bridge"},
	{ INTEL, 0x1222, INTEL_NAME, "82092AA EIDE"},
	{ INTEL, 0x1223, INTEL_NAME, "SAA7116"},
	{ INTEL, 0x1226, INTEL_NAME, "82596"},
	{ INTEL, 0x1227, INTEL_NAME, "82865"},
	{ INTEL, 0x1229, INTEL_NAME, "82557"},
	{ INTEL, 0x122d, INTEL_NAME, "82437"},
	{ INTEL, 0x122e, INTEL_NAME, "82371 Triton PIIX"},
	{ INTEL, 0x1230, INTEL_NAME, "82371 Triton PIIX"},
	{ INTEL, 0x1234, INTEL_NAME, "430MX - 82371MX MPIIX"},
	{ INTEL, 0x1235, INTEL_NAME, "430MX - 82437MX MTSC"},
	{ INTEL, 0x1237, INTEL_NAME, "82441FX Natoma"},
	{ INTEL, 0x124b, INTEL_NAME, "82380FB Mobile"},
	{ INTEL, 0x1250, INTEL_NAME, "82439HX Triton II"},
	{ INTEL, 0x7000, INTEL_NAME, "82371SB PIIX3 ISA"},
	{ INTEL, 0x7010, INTEL_NAME, "82371SB PIIX3 IDE"},
	{ INTEL, 0x7020, INTEL_NAME, "82371SB PIIX3 USB"},
	{ INTEL, 0x7030, INTEL_NAME, "82437VX Triton II"},
	{ INTEL, 0x7100, INTEL_NAME, "82439TX"},
	{ INTEL, 0x7110, INTEL_NAME, "82371AB PIIX4 ISA"},
	{ INTEL, 0x7111, INTEL_NAME, "82371AB PIIX4 IDE"},
	{ INTEL, 0x7112, INTEL_NAME, "82371AB PIIX4 USB"},
	{ INTEL, 0x7113, INTEL_NAME, "82371AB PIIX4 ACPI"},
	{ INTEL, 0x7180, INTEL_NAME, "440LX - 82443LX PAC Host"},
	{ INTEL, 0x7181, INTEL_NAME, "440LX - 82443LX PAC AGP"},
	{ INTEL, 0x7190, INTEL_NAME, "440BX - 82443BX Host"},
	{ INTEL, 0x7191, INTEL_NAME, "440BX - 82443BX AGP"},
	{ INTEL, 0x7192, INTEL_NAME, "440BX - 82443BX Host {no AGP}"},
	{ INTEL, 0x71A0, INTEL_NAME, "440GX - 82443GX Host"},
	{ INTEL, 0x71A1, INTEL_NAME, "440GX - 82443GX AGP"},
	{ INTEL, 0x71A2, INTEL_NAME, "440GX - 82443GX Host {no AGP}"},
	{ INTEL, 0x84c4, INTEL_NAME, "Orion P6"},
 	{ INTEL, 0x84c5, INTEL_NAME, "82450GX Orion P6"},
 	{ VENDOR_UNKNOWN, 0x0000, UNKNOWN_NAME, "Unknown"}
};

struct PCI_BASECLASSNAME pci_baseclassnames[] = {
	{ 0x01, "Mass storage controller" },
	{ 0x02, "Network controller" },
	{ 0x03, "Display controller" },
	{ 0x04, "Multimedia device" },
	{ 0x05, "Memory controller" },
	{ 0x06, "Bridge device" },
	{ 0x07, "Simple comm. controller" },
	{ 0x08, "Base system peripheral" },
	{ 0x09, "Input device" },
	{ 0x0A, "Docking station" },
	{ 0x0B, "Processor" },
	{ 0x0C, "Serial bus controller" },
	{ 0x0D, "Wireless controller" },
	{ 0x0E, "Intelligent I/O controller" },
	{ 0x0F, "Satellite comm. controller" },
	{ 0x10, "Encryption/decryption controller" },
	{ 0x11, "Data acquisition controller" },
	{ 0xFF, "Unknown class" },
};

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

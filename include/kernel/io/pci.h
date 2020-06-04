#ifndef _KERNEL_IO_PCI_H_
#define _KERNEL_IO_PCI_H_

#include <sys/types.h>

#define PCI_MAXDEVICE		32
#define PCI_MAXFUNCTION		8

#define PCI_VENDORID		0
#define PCI_DEVICEID		2
#define PCI_SUBCLASS		0x0A
#define PCI_BASECLASS		0x0B
#define PCI_HEADERTYPE		0x0E
#define PCI_SUBSYSVENDOR	0x2C
#define PCI_SUBSYS			0x2E
#define PCI_IRQ				0x3C

#define CIRRUS				0x1013 // Cirrus Logic
#define REALTEK				0x10EC // Realtek Semiconductor Co., Ltd.
#define INTEL				0x8086
#define VENDOR_UNKNOWN		0x0000

#define CIRRUS_NAME		"Cirrus Logic"
#define REALTEK_NAME	"Realtek Semiconductor"
#define INTEL_NAME		"Intel"
#define UNKNOWN_NAME	"Unknown"

struct PCI_DEVICENAME
{
	const WORD vendor_id;
	const WORD device_id;
	const char * vendor_name;
	const char * device_name;
};

struct PCI_BASECLASSNAME
{
	const BYTE base_class;
	const char * name;
};

struct PCI_DEVICEINFO
{
    WORD vendor_id;
    WORD device_id;
    WORD subsys_vendor;
    WORD subsys;	
	BYTE header_type;
    BYTE sub_class;
    BYTE base_class;
    BYTE irq;
	const char * vendor_name;
	const char * device_name;
	const char * class_name;
    struct PCI_DEVICEINFO * prev;
};

struct conf
{
    BYTE reg:8;
    BYTE func:3;
    BYTE dev:5;
    BYTE bus:8;
    BYTE rsvd:7;
    BYTE enable:1;
};

struct PCI_CONF
{
	union
	{
		struct conf bits;
		DWORD data;
	};
};

int pci_init( void );

#endif

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

struct PCI_BASECLASSNAME
{
	const BYTE base_class;
	const char * name;
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

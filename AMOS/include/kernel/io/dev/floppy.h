#ifndef _KERNEL_IO_DEV_FLOPPY_H_
#define _KERNEL_IO_DEV_FLOPPY_H_

#include <sys/types.h>

/*
 * Reference: http://bos.asmhackers.net/docs/floppy/docs/FLOPPY2.TXT
 * 
 * Format   Size   Cyls   Heads  Sec/Trk
 * 360K     5 1/4   40      2       9
 * 1.2M     5 1/4   80      2      15
 * 720K     3 1/2   80      2       9
 * 1.44M    3 1/2   80      2      18
 * 2.88M    3 1/2   80      2      36
 * 
 */
 
struct FLOPPY_FORMAT
{
	int cylinders;
	int heads;
	int tracks;	
};

struct FLOPPY_DRIVE
{
	BYTE type;
	struct FLOPPY_FORMAT format;
};

enum FLOPPY_TYPE
{
	TUNUSED=1,	// not available
	TYPE_A,		// 360KB 5.25"
	TYPE_B,		// 1.2MB 5.25"
	TYPE_C,		// 720KB 3.5"
	TYPE_D,		// 1.44MB 3.5"
	TYPE_E		// 2.88MB 3.5"
};

void floppy_init();

#endif

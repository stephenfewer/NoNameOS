#ifndef _KERNEL_IO_DEV_KEYBOARD_H_
#define _KERNEL_IO_DEV_KEYBOARD_H_

#include <sys/types.h>
#include <kernel/io/io.h>

#define KEYBOARD_DATAREG		0x60
#define KEYBOARD_CONTROLREG		0x64

int keyboard_init();

#endif

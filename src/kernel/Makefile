MAKEFILE=Makefile

TARGET_ELF=kernel.elf

TARGET_DIR=../../bin/kernel/

INCLUDE_DIR=../../include/

AMOS_LIBC=./../../bin/lib/libc.a

MM=mm
PM=pm
IO=io
FS=fs
DEV=dev
SYNC=sync

CC=gcc
LD=ld-elf
AS=nasmw
RM=del

ASFLAGS=-f elf
LDFLAGS=-T kernel.ld -Map $(TARGET_DIR)kernel.map -nostdlib
CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -nostdinc -I $(INCLUDE_DIR)

OBJ=loader.o isr.o kernel.o interrupt.o syscall.o kprintf.o debug.o
	
all: $(TARGET_ELF)

$(TARGET_ELF): $(OBJ)
	$(MAKE) -C $(FS)
	$(MAKE) -C $(MM)
	$(MAKE) -C $(PM)
	$(MAKE) -C $(IO)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$(TARGET_ELF) $(OBJ) ./$(MM)/*.o ./$(PM)/*.o ./$(PM)/$(SYNC)/*.o ./$(IO)/*.o ./$(IO)/$(DEV)/*.o ./$(FS)/*.o $(AMOS_LIBC)
	
%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<
	
%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(MM) clean
	$(MAKE) -C $(PM) clean
	$(MAKE) -C $(IO) clean
	$(MAKE) -C $(FS) clean

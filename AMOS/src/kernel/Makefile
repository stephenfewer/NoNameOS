MAKEFILE=Makefile

TARGET_ELF=kernel.elf
TARGET_COFF=kernel.coff

TARGET_DIR=../../bin/

INCLUDE_DIR=../../include/

MM=mm
MT=tasking
IO=io
FS=fs
LIB=lib
DEV=dev

CC=gcc
LD=ld
AS=nasmw
RM=del
STRIP=strip
OBJCOPY=objcopy-elf

ASFLAGS=-f coff
LDFLAGS=-T kernel.ld -Map $(TARGET_DIR)kernel.map -nostdlib
CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -finline-functions -nostdinc -I $(INCLUDE_DIR)
OBJCOPY_FLAGS=-I coff-i386 -O elf32-i386

OBJ=loader.o kernel.o gdt.o idt.o irq.o isr.o kprintf.o debug.o
	
all: $(TARGET_ELF)

$(TARGET_ELF): $(OBJ)
	$(MAKE) -C $(MM)
	$(MAKE) -C $(MT)
	$(MAKE) -C $(IO)
	$(MAKE) -C $(LIB)
	$(MAKE) -C $(FS)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$(TARGET_COFF) $(OBJ) ./$(MM)/*.o ./$(MT)/*.o ./$(IO)/*.o ./$(IO)/$(DEV)/*.o ./$(FS)/*.o ./$(LIB)/*.o
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(TARGET_DIR)$(TARGET_COFF) $(TARGET_DIR)$(TARGET_ELF)
	
%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<
	
%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(MM) clean
	$(MAKE) -C $(MT) clean
	$(MAKE) -C $(IO) clean
	$(MAKE) -C $(FS) clean
	$(MAKE) -C $(LIB) clean

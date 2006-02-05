MAKEFILE=Makefile

TARGET_ELF=kernel.elf
TARGET_COFF=kernel.coff

TARGET_DIR=../../bin/kernel/

INCLUDE_DIR=../../include/

MM=mm
PM=pm
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

OBJ=loader.o kernel.o idt.o irq.o isr.o kprintf.o
	
all: $(TARGET_ELF)

$(TARGET_ELF): $(OBJ)
	$(MAKE) -C $(FS)
	$(MAKE) -C $(MM)
	$(MAKE) -C $(PM)
	$(MAKE) -C $(IO)
	$(MAKE) -C $(LIB)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$(TARGET_COFF) $(OBJ) ./$(MM)/*.o ./$(PM)/*.o ./$(IO)/*.o ./$(IO)/$(DEV)/*.o ./$(FS)/*.o ./$(LIB)/*.o
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(TARGET_DIR)$(TARGET_COFF) $(TARGET_DIR)$(TARGET_ELF)
	
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
	$(MAKE) -C $(LIB) clean

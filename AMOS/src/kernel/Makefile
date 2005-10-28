MAKEFILE=Makefile

TARGET_ELF=kernel.elf
TARGET_COFF=kernel.coff

TARGET_DIR=../../bin/

INCLUDE_DIR=../../include/

MEMORY_MANAGER=mm
MULTI_TASKING=tasking
IO=io
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

OBJ=loader.o kernel.o gdt.o idt.o irq.o isr.o debug.o kprintf.o
	
all: $(TARGET_ELF)

$(TARGET_ELF): $(OBJ)
	$(MAKE) -C $(MEMORY_MANAGER)
	$(MAKE) -C $(MULTI_TASKING)
	$(MAKE) -C $(IO)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$(TARGET_COFF) $(OBJ) ./$(MEMORY_MANAGER)/*.o ./$(MULTI_TASKING)/*.o ./$(IO)/*.o ./$(IO)/$(DEV)/*.o
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(TARGET_DIR)$(TARGET_COFF) $(TARGET_DIR)$(TARGET_ELF)
	
%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<
	
%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(MEMORY_MANAGER) clean
	$(MAKE) -C $(MULTI_TASKING) clean
	$(MAKE) -C $(IO) clean

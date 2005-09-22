MAKEFILE=Makefile

TARGET=kernel.bin

TARGET_DIR=../../bin/

INCLUDE_DIR=../../include/

MEMORY_MANAGER=mm

CC=gcc
LD=ld
AS=nasmw
RM=del
STRIP=strip

ASFLAGS=-f coff
LDFLAGS=-T kernel.ld -Map $(TARGET_DIR)kernel.map -nostdlib
CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -finline-functions -nostdinc -I $(INCLUDE_DIR)

OBJ=loader.o kernel.o console.o gdt.o idt.o irq.o isr.o
	
all: $(TARGET)

$(TARGET): $(OBJ)
	$(MAKE) -C $(MEMORY_MANAGER)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$@ $(OBJ) ./$(MEMORY_MANAGER)/*.o
	
	
%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<
	
%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(MEMORY_MANAGER) clean

MAKEFILE=Makefile

TARGET=kernel.bin

TARGET_DIR=../../bin/

INCLUDE_DIR=../../include/

MM=mm

CC=gcc
LD=ld
AS=nasmw
RM=del

ASFLAGS=-f aout
LDFLAGS=-T link.ld
CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -finline-functions -nostdinc -I $(INCLUDE_DIR)

OBJ=loader.o kernel.o console.o gdt.o idt.o irq.o isr.o
	
all: $(TARGET)

$(TARGET): $(OBJ)
	$(MAKE) -C $(MM)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$@ $(OBJ) ./$(MM)/*.o

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<
	
%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(MM) clean

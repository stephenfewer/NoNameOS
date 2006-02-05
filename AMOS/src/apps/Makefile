MAKEFILE=Makefile

TARGET_ELF=test.elf
TARGET_COFF=test.coff

TARGET_DIR=../../bin/apps/

INCLUDE_DIR=../../include/

CC=gcc
LD=ld
RM=del
OBJCOPY=objcopy-elf

LDFLAGS=--oformat coff-go32 -nostdlib
CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -finline-functions -nostdinc -I $(INCLUDE_DIR)
OBJCOPY_FLAGS=-I coff-i386 -O elf32-i386

OBJ=test.o
	
all: $(TARGET_ELF)

$(TARGET_ELF): $(OBJ)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$(TARGET_COFF) $(OBJ)
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(TARGET_DIR)$(TARGET_COFF) $(TARGET_DIR)$(TARGET_ELF)
	
%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o
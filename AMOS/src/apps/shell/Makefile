MAKEFILE=Makefile

TARGET_ELF=shell.elf

TARGET_DIR=../../../bin/apps/

INCLUDE_DIR=../../../include/

CC=gcc
LD=ld
RM=del

LDFLAGS=--oformat elf32-i386 -nostdlib
CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -finline-functions -nostdinc -I $(INCLUDE_DIR)

OBJ=shell.o tinysh.o
	
all: $(TARGET_ELF)

$(TARGET_ELF): $(OBJ)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$(TARGET_ELF) $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o
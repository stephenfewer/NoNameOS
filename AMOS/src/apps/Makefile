MAKEFILE=Makefile

TARGET_ELF=test.bin

TARGET_DIR=../../bin/apps/

INCLUDE_DIR=../../include/

CC=gcc
LD=ld
RM=del

LDFLAGS=--oformat binary -Ttext 0x10000000
CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -nostdinc -I $(INCLUDE_DIR)

OBJ=test.o
	
all: $(TARGET_ELF)

$(TARGET_ELF): $(OBJ)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$(TARGET_ELF) $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o
MAKEFILE=Makefile

INCLUDE_DIR=../../../include/

SYNC=sync

CC=gcc
RM=del

CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -nostdinc -I $(INCLUDE_DIR)

OBJ=scheduler.o process.o
	
all: $(OBJ)
	$(MAKE) -C $(SYNC)

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(SYNC) clean
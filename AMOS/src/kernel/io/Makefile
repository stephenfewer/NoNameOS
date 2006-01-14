MAKEFILE=Makefile

INCLUDE_DIR=../../../include/

DEV=dev

CC=gcc
RM=del

CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -finline-functions -nostdinc -I $(INCLUDE_DIR)

OBJ=io.o

all: $(OBJ)
	$(MAKE) -C $(DEV)
	
%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(DEV) clean

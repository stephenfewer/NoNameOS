MAKEFILE=Makefile

LIBC=libc

TARGET_DIR=.

INCLUDE_DIR=../../include/

CC=gcc
RM=del

CCFLAGS=-Wall -O -nostdlib -fno-builtin -nostdinc -I $(INCLUDE_DIR)

OBJ=amos.o
	
all: $(OBJ)
	$(MAKE) -C $(LIBC)

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(LIBC) clean

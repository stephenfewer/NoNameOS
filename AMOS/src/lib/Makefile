MAKEFILE=Makefile

TARGET_DIR=.

INCLUDE_DIR=../../include/

CC=gcc
RM=del

CCFLAGS=-Wall -O -nostdlib -fno-builtin -nostdinc -I $(INCLUDE_DIR)

OBJ=amos.o printf.o string.o
	
all: $(OBJ)
%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o
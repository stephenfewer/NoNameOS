MAKEFILE=Makefile

TARGET_LIB=libc.a

TARGET_DIR=../../../bin/lib/

INCLUDE_DIR=../../../include/

CC=gcc
RM=del
AR=ar

ARFLAGS=rcs
CCFLAGS=-Wall -O -nostdlib -fno-builtin -nostdinc -I $(INCLUDE_DIR)

OBJ=ctype.o stdio.o stdlib.o string.o
	
all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET_DIR)$(TARGET_LIB) $(OBJ) ../amos.o

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o
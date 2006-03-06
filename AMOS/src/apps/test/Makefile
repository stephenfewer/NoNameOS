MAKEFILE=Makefile

TARGET_BIN=test.bin

TARGET_DIR=../../../bin/apps/

INCLUDE_DIR=../../../include/

CC=gcc
LD=ld
RM=del

AMOS_LIB=../../lib/*.o

LDFLAGS=-T ../apps.ld -s -nostdlib -Map $(TARGET_DIR)test.map
CCFLAGS=-Wall -O -nostdlib -fno-builtin -nostdinc -I $(INCLUDE_DIR)

OBJ=test.o
	
all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJ)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$(TARGET_BIN) $(OBJ) $(AMOS_LIB)

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o

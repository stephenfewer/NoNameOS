MAKEFILE=Makefile

TARGET_BIN=shell.bin

TARGET_DIR=../../../bin/apps/

INCLUDE_DIR=../../../include/

AMOS_LIBC=../../../bin/lib/libc.a

CC=gcc
LD=ld
RM=del

LDFLAGS=-T ../apps.ld -s -nostdlib -Map $(TARGET_DIR)shell.map
CCFLAGS=-Wall -O -nostdlib -fno-builtin -nostdinc -I $(INCLUDE_DIR)

OBJ=main.o tinysh.o
	
all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJ)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$(TARGET_BIN) $(OBJ) $(AMOS_LIBC)

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o

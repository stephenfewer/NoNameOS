MAKEFILE=Makefile

export CC=gcc
export LD=ld
export RM=rm
export AR=ar
export AS=nasm

export CCFLAGS=-m32 -Wall -O -nostdlib -nostdinc -fno-builtin -fno-pie

LIB=src/lib
KERNEL=src/kernel
APPS=src/apps
BIN=bin

all: image

lib:
	$(MAKE) -C $(LIB)

kernel:
	$(MAKE) -C $(KERNEL)

apps:
	$(MAKE) -C $(APPS)

clean:
	$(MAKE) -C $(LIB) clean
	$(MAKE) -C $(KERNEL) clean
	$(MAKE) -C $(APPS) clean
	$(MAKE) -C $(BIN) clean
	
image: lib kernel apps
	$(MAKE) -C $(BIN)

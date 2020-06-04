MAKEFILE=Makefile

TARGET_ELF=kernel.elf

TARGET_DIR=../../bin/kernel/

INCLUDE_DIR=../../include/

AMOS_LIBC=./../../bin/lib/libc.a

MM=mm
PM=pm
IO=io
FS=fs
DEV=dev
SYNC=sync

ASFLAGS=-f elf32
LDFLAGS=-no-pie -m32 -m elf_i386 -T kernel.ld -Map $(TARGET_DIR)kernel.map -nostdlib -nostdinc

OBJ=loader.o isr.o kernel.o interrupt.o syscall.o kprintf.o debug.o
	
all: $(TARGET_ELF)

$(TARGET_ELF): $(OBJ)
	$(MAKE) -C $(FS)
	$(MAKE) -C $(MM)
	$(MAKE) -C $(PM)
	$(MAKE) -C $(IO)
	$(LD) $(LDFLAGS) -o $(TARGET_DIR)$(TARGET_ELF) $(OBJ) ./$(MM)/*.o ./$(PM)/*.o ./$(PM)/$(SYNC)/*.o ./$(IO)/*.o ./$(IO)/$(DEV)/*.o ./$(FS)/*.o $(AMOS_LIBC)
	
%.o: %.c
	$(CC) $(CCFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<
	
%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(MM) clean
	$(MAKE) -C $(PM) clean
	$(MAKE) -C $(IO) clean
	$(MAKE) -C $(FS) clean

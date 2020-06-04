MAKEFILE=Makefile

TARGET=NoNameOS.vfd

COPY=cp
MCOPY=mcopy
MMD=mmd
RM=rm

all: $(TARGET)
	$(MCOPY) -i $(TARGET) -D o ./kernel/kernel.elf ::/amos
	$(MCOPY) -i $(TARGET) -D o ./apps/init.bin ::/amos
	$(MCOPY) -i $(TARGET) -D o ./apps/shell.bin ::/amos
	$(MCOPY) -i $(TARGET) -D o ./apps/test.bin ::/amos
	$(MCOPY) -i $(TARGET) -D o ./apps/hanoi.bin ::/amos
	$(MCOPY) -i $(TARGET) -D o ./lib/libc.a ::/amos/lib

$(TARGET):
	$(COPY) grub.ima $(TARGET)
	$(MCOPY)  -i $(TARGET) -D o ./menu.cfg ::/boot
	$(MMD) -i $(TARGET) ::/amos
	$(MMD) -i $(TARGET) ::/amos/lib

clean:
	$(RM) $(TARGET)
	$(RM) ./kernel/kernel.*
	$(RM) ./apps/*.bin
	$(RM) ./apps/*.map
	$(RM) ./lib/*.a
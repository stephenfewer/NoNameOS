MAKEFILE=Makefile

TARGET=amos.ima

COPY=copy
RM=del

all: $(TARGET)
	..\tools\mcopy -o .\kernel.bin a:\boot

$(TARGET):
	$(COPY) .\grub.ima .\$(TARGET)
	..\tools\mcopy -o .\menu.cfg a:\boot
	
clean:
	$(RM) $(TARGET)
	$(RM) *.bin
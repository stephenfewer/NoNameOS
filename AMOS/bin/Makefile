MAKEFILE=Makefile

TARGET=amos.ima

COPY=copy
MCOPY=..\tools\mcopy
RM=del

all: $(TARGET)
	$(MCOPY) -o .\kernel.bin a:\boot

$(TARGET):
	$(COPY) .\grub.ima .\$(TARGET)
	$(MCOPY) -o .\menu.cfg a:\boot
	
clean:
	$(RM) $(TARGET)
	$(RM) *.bin
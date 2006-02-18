MAKEFILE=Makefile

TARGET=amos.ima

KERNEL_ELF=.\kernel\kernel.elf
KERNEL_TXT=.\kernel\kernel.txt
KERNEL_MAP=.\kernel\kernel.map

IMAGE=grub.ima
IMAGE_DIR=a:\boot
IMAGE_MENU=menu.cfg

COPY=copy
MCOPY=..\tools\mcopy
RM=del
OBJDUMP=objdump-elf

all: $(TARGET)
	$(MCOPY) -o $(KERNEL_ELF) $(IMAGE_DIR)
	$(MCOPY) -o .\apps\test.bin $(IMAGE_DIR)
	
$(TARGET):
	$(COPY) .\$(IMAGE) .\$(TARGET)
	$(MCOPY) -o .\$(IMAGE_MENU) $(IMAGE_DIR)

clean:
	$(RM) $(TARGET)
	$(RM) $(KERNEL_ELF)
	$(RM) $(KERNEL_TXT)
	$(RM) $(KERNEL_MAP)


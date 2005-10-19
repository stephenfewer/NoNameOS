MAKEFILE=Makefile

TARGET=amos.ima

KERNEL_ELF=kernel.elf
KERNEL_COFF=kernel.coff
KERNEL_TXT=kernel.txt

IMAGE=grub.ima
IMAGE_DIR=a:\boot
IMAGE_MENU=menu.cfg

COPY=copy
MCOPY=..\tools\mcopy
RM=del
OBJDUMP=objdump-elf

all: $(TARGET)
	$(MCOPY) -o .\$(KERNEL_ELF) $(IMAGE_DIR)

$(TARGET):
	$(COPY) .\$(IMAGE) .\$(TARGET)
	$(MCOPY) -o .\$(IMAGE_MENU) $(IMAGE_DIR)
	
clean:
	$(RM) $(TARGET)
	$(RM) $(KERNEL_ELF)
	$(RM) $(KERNEL_COFF)
	$(RM) $(KERNEL_TXT)

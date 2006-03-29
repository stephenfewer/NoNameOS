MAKEFILE=Makefile

TARGET=amos.ima

KERNEL_ELF=.\kernel\kernel.elf
KERNEL_TXT=.\kernel\kernel.txt
KERNEL_MAP=.\kernel\kernel.map

IMAGE=grub.ima
IMAGE_DRIVE=a:
IMAGE_MENU=menu.cfg

COPY=copy
MCOPY=..\tools\mcopy -o
MMD=..\tools\mmd
RM=del
OBJDUMP=objdump-elf

all: $(TARGET)
	$(MCOPY) $(KERNEL_ELF) $(IMAGE_DRIVE)\amos
	$(MCOPY) .\apps\test.bin $(IMAGE_DRIVE)\amos
	$(MCOPY) .\apps\shell.bin $(IMAGE_DRIVE)\amos
	$(MCOPY) .\apps\init.bin $(IMAGE_DRIVE)\amos
	$(MCOPY) .\a.txt $(IMAGE_DRIVE)\amos

$(TARGET):
	$(COPY) .\$(IMAGE) .\$(TARGET)
	$(MCOPY) .\$(IMAGE_MENU) $(IMAGE_DRIVE)\boot
	$(MMD) $(IMAGE_DRIVE)\amos

clean:
	$(RM) $(TARGET)
	$(RM) $(KERNEL_ELF)
	$(RM) $(KERNEL_TXT)
	$(RM) $(KERNEL_MAP)


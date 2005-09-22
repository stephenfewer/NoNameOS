objcopy-elf -I coff-i386 -O elf32-i386 kernel.bin kernel.elf
del kernel.bin
rename kernel.elf kernel.bin
..\tools\mcopy -o kernel.bin a:\boot
objdump-elf -d kernel.bin > kernel.txt
objdump-elf -h kernel.bin
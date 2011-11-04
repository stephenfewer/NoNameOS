About
=====

NoNameOS (Originally called AMOS and written in 2005/2006) is a tiny Operating System for the x86 architecture. It is an educational system with a monolithic kernel design and a clean efficient implementation. Features include a simple virtual memory manager, a file system and fully pre-emptive multitasking. 

Tool Chain
==========

To build NoNameOS you will need DJGPP and NASM. 

Building
========

In order to build the NoNameOS source code you will need the tools mentioned above. Enter the root directory of the code and run make. It should be that simple. Make sure you have your environment variables set up correctly, e.g.

> set PATH=C:\DJGPP\bin;%PATH set DJGPP=C:\DJGPP\djgpp.env

You will also need to set the MTOOLSRC environment variable in order for the mcopy command to work. The mtools.conf file is located in the tools directory of the NoNameOS source code distribution, along with the mcopy utility

> set MTOOLSRC=C:\NoNameOS\tools\mtools.conf

If you are building this on Windows you will need to install binutils for DJGPP in order to build ELF files.

The linker will place the finished binary kernel.bin in the bin directory.

The make process will create the ELF kernel and place kernel.elf in the disk image NoNameOS.ima. GRUB is already configured to boot the kernel. The make process will also create the user shell application shell.bin and a test application test.bin, both are flat binary format.

You can now use bochs to boot off the disk image, the bochsrc.bxrc config script should be fine but go over it to make sure. In Eclipse you can set up an external tool to run the run_bochs.bat file to automate this, otherwise double click on the bochsrc.bxrc config script or run it from the command line, e.g.

> C:\NoNameOS\bin\bochs -q -f bochsrc.bxrc

You can also run it in qemu using the run_qemu.bat file also located in the bin directory. You may need to edit the path in both of these scripts to suit your system.

You should now be running NoNameOS! 

License
=======

The source code is available under the GPLv3 license, please see the included file gpl-3.0.txt for details. 
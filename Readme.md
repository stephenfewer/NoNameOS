About
=====

NoNameOS (Originally called AMOS and written in 2005/2006) is a tiny Operating System for the x86 architecture. It is an educational system with a monolithic kernel design and a clean efficient implementation. Features include a simple virtual memory manager, a file system and fully pre-emptive multitasking. 

Tool Chain
==========

To build NoNameOS you will need gcc, nasm and mtools and a Linux like environment. On Ubuntu install via: 

`apt install gcc nasm mtools`

Building
========

To build simply run make, an image `NoNameOS.vfd` will be created in the bin directory

You can no run via QEMU, HyperV or similar, e.g:

`qemu-system-x86_64 -curses -fda ./bin/NoNameOS.vfd`

You should now be running NoNameOS! 

License
=======

The source code is available under the GPLv3 license, please see the included file gpl-3.0.txt for details. 
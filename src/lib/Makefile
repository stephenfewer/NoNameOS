MAKEFILE=Makefile

LIBC=libc

TARGET_DIR=.

INCLUDE_DIR=../../include/

OBJ=amos.o
	
all: $(OBJ)
	$(MAKE) -C $(LIBC)

%.o: %.c
	$(CC) $(CCFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(LIBC) clean

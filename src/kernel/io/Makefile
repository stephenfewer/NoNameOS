MAKEFILE=Makefile

INCLUDE_DIR=../../../include/

DEV=dev

OBJ=io.o port.o pci.o

all: $(OBJ)
	$(MAKE) -C $(DEV)
	
%.o: %.c
	$(CC) $(CCFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(DEV) clean

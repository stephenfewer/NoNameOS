MAKEFILE=Makefile

INCLUDE_DIR=../../../../include/

OBJ=console.o keyboard.o floppy.o bitbucket.o

all: $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<

clean:
	$(RM) *.o
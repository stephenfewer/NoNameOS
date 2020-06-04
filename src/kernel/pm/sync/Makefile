MAKEFILE=Makefile

INCLUDE_DIR=../../../../include/

OBJ=mutex.o

all: $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<

clean:
	$(RM) *.o
MAKEFILE=Makefile

INCLUDE_DIR=../../../include/

OBJ=vfs.o fat.o dfs.o

all: $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<

clean:
	$(RM) *.o
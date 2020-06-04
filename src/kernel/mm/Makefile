MAKEFILE=Makefile

INCLUDE_DIR=../../../include/

OBJ=paging.o segmentation.o physical.o mm.o dma.o
	
all: $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<

clean:
	$(RM) *.o
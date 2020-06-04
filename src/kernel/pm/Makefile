MAKEFILE=Makefile

INCLUDE_DIR=../../../include/

SYNC=sync

OBJ=scheduler.o process.o
	
all: $(OBJ)
	$(MAKE) -C $(SYNC)

%.o: %.c
	$(CC) $(CCFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<

clean:
	$(RM) *.o
	$(MAKE) -C $(SYNC) clean
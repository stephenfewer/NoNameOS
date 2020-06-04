MAKEFILE=Makefile

TARGET_LIB=libc.a

TARGET_DIR=../../../bin/lib/

INCLUDE_DIR=../../../include/

ARFLAGS=rcs

OBJ=ctype.o stdio.o stdlib.o string.o
	
all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET_DIR)$(TARGET_LIB) $(OBJ) ../amos.o

%.o: %.c
	$(CC) $(CCFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<

clean:
	$(RM) *.o

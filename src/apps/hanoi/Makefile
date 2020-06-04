MAKEFILE=Makefile

TARGET_BIN=hanoi.bin

OBJ=hanoi.o
	
all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJ)
	$(LD) $(LDFLAGS) -Map $(TARGET_DIR)hanoi.map -o $(TARGET_DIR)$(TARGET_BIN) $(OBJ) $(AMOS_LIBC)

%.o: %.c
	$(CC) $(CCFLAGS) -ffunction-sections -I $(INCLUDE_DIR) -c -o $@ $<

clean:
	$(RM) *.o

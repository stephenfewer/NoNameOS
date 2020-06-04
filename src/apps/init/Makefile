MAKEFILE=Makefile

TARGET_BIN=init.bin

OBJ=init.o
	
all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJ)
	$(LD) $(LDFLAGS) -Map $(TARGET_DIR)init.map -o $(TARGET_DIR)$(TARGET_BIN) $(OBJ) $(AMOS_LIBC)

%.o: %.c
	$(CC) $(CCFLAGS) -ffunction-sections -I $(INCLUDE_DIR) -c -o $@ $<

clean:
	$(RM) *.o

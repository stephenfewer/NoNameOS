MAKEFILE=Makefile

TARGET_BIN=shell.bin

OBJ=main.o tinysh.o
	
all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJ)
	$(LD) $(LDFLAGS) -Map $(TARGET_DIR)shell.map -o $(TARGET_DIR)$(TARGET_BIN) $(OBJ) $(AMOS_LIBC)

%.o: %.c
	$(CC) $(CCFLAGS) -ffunction-sections -I $(INCLUDE_DIR) -c -o $@ $<

clean:
	$(RM) *.o

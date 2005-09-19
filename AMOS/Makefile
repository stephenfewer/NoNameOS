KERNEL=src/kernel
BIN=bin

all: image

kernel:
	$(MAKE) -C $(KERNEL)
	
clean:
	$(MAKE) -C $(KERNEL) clean
	$(MAKE) -C $(BIN) clean
	
image: kernel
	$(MAKE) -C $(BIN)
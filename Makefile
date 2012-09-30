MAKEFILE=Makefile

LIB=src/lib
KERNEL=src/kernel
APPS=src/apps
BIN=bin

all: image

lib:
	$(MAKE) -C $(LIB)

kernel:
	$(MAKE) -C $(KERNEL)

apps:
	$(MAKE) -C $(APPS)

clean:
	$(MAKE) -C $(LIB) clean
	$(MAKE) -C $(KERNEL) clean
	$(MAKE) -C $(APPS) clean
	$(MAKE) -C $(BIN) clean
	
image: lib kernel apps
	$(MAKE) -C $(BIN)

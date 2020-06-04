MAKEFILE=Makefile

export TARGET_DIR=../../../bin/apps/
export INCLUDE_DIR=../../../include/
export AMOS_LIBC=../../../bin/lib/libc.a

export LDFLAGS=-no-pie -m32 -m elf_i386 -T ../apps.ld --strip-all -nostdlib -nostdinc

AMOS_INIT=init
AMOS_SHELL=shell
TEST=test
HANOI=hanoi

all: app_init app_shell app_test app_hanoi

app_init:
	$(MAKE) -C $(AMOS_INIT)
	
app_shell:
	$(MAKE) -C $(AMOS_SHELL)
	
app_test:
	$(MAKE) -C $(TEST)
	
app_hanoi:
	$(MAKE) -C $(HANOI)

clean:
	$(MAKE) -C $(AMOS_INIT) clean
	$(MAKE) -C $(AMOS_SHELL) clean
	$(MAKE) -C $(TEST) clean
	$(MAKE) -C $(HANOI) clean

MAKEFILE=Makefile

AMOS_INIT=init
AMOS_SHELL=shell
TEST=test

all: app_init app_shell app_test

app_init:
	$(MAKE) -C $(AMOS_INIT)
	
app_shell:
	$(MAKE) -C $(AMOS_SHELL)
	
app_test:
	$(MAKE) -C $(TEST)
	
clean:
	$(MAKE) -C $(AMOS_INIT) clean
	$(MAKE) -C $(AMOS_SHELL) clean
	$(MAKE) -C $(TEST) clean

MAKEFILE=Makefile

AMOS_SHELL=shell
TEST=test

all: app_shell app_test

app_shell:
	$(MAKE) -C $(AMOS_SHELL)
	
app_test:
	$(MAKE) -C $(TEST)
	
clean:
	$(MAKE) -C $(AMOS_SHELL) clean
	$(MAKE) -C $(TEST) clean

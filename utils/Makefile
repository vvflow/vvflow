utils:= $(shell ls -d */) 
all clean install uninstall:
	$(patsubst  %, make -C % $@;, $(utils))


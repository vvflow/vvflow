TARGETS_INSTALL   += vvscripts_install
TARGETS_UNINSTALL += vvscripts_uninstall
VVSCRIPTS = gpquick vvencode
VVSCRIPTS += vvawk.avg vvawk.mavg vvawk.mse vvawk.zeros

vvscripts_install: $(VVSCRIPTS) | $(PREFIX)/bin
	$(foreach f,$^,\
		cp $(f) -t $(PREFIX)/bin${\n}\
	)

vvscripts_uninstall:
	$(foreach f,$(VVSCRIPTS),\
		rm -f $(PREFIX)/bin/$(f)${\n}\
	)
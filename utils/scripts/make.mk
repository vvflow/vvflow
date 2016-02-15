TARGETS_INSTALL   += vvscripts_install
TARGETS_UNINSTALL += vvscripts_uninstall

vvscripts_install: avg.awk movavg.awk stdev.awk zeros.awk gpquick vvencode | $(PREFIX)/bin
	cp $^ $(PREFIX)/bin

vvscripts_uninstall:
	rm -f $(PREFIX)/bin/avg.awk
	rm -f $(PREFIX)/bin/movavg.awk
	rm -f $(PREFIX)/bin/stdev.awk
	rm -f $(PREFIX)/bin/zeros.awk
	rm -f $(PREFIX)/bin/gpquick
	rm -f $(PREFIX)/bin/vvencode


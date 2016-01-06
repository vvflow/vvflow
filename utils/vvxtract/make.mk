TARGETS_ALL       += bin/vvxtract
TARGETS_INSTALL   += vvxtract_install
TARGETS_UNINSTALL += vvxtract_uninstall

bin/vvxtract: bin/libvvhd.so
bin/vvxtract: vvxtract.cpp
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -lvvhd -o ./$@

vvxtract_install: h5xtract | $(PREFIX)/bin
	cp ./bin/vvxtract -t $(PREFIX)/bin/
	cp $^ -t $(PREFIX)/bin/

vvxtract_uninstall:
	rm -f $(PREFIX)/bin/vvxtract
	rm -f $(PREFIX)/bin/h5xtract



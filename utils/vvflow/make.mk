TARGETS_ALL       += bin/vvflow
TARGETS_INSTALL   += vvflow_install
TARGETS_UNINSTALL += vvflow_uninstall

bin/vvflow: bin/libvvhd.so
bin/vvflow: vvflow.cpp
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -lvvhd -o ./$@

vvflow_install: | $(PREFIX)/bin
	cp ./bin/vvflow -t $(PREFIX)/bin/

vvflow_uninstall:
	rm -f $(PREFIX)/bin/vvflow



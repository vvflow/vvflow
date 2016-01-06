TARGETS_ALL       += bin/vvcompose
TARGETS_INSTALL   += vvcompose_install vvcompose_completion_install
TARGETS_UNINSTALL += vvcompose_uninstall

bin/vvcompose: bin/libvvhd.so
bin/vvcompose: vvcompose.cpp
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -lvvhd -o ./$@

vvcompose_install: | $(PREFIX)/bin
	cp ./bin/vvcompose -t $(PREFIX)/bin/
vvcompose_completion_install: completion_vvcompose | $(PREFIX)/share/vvhd/bash_completion.d
	cp $^ -t $(PREFIX)/share/vvhd/bash_completion.d/

vvcompose_uninstall:
	rm -f $(PREFIX)/bin/vvcompose
	rm -f $(PREFIX)/share/vvhd/bash_completion.d/completion_vvcompose

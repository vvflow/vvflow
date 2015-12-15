include ../../vvhd.mk

all: vvcompose

vvcompose: vvcompose.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -std=c++0x -Wl,-Bstatic -lvvhd -Wl,-Bdynamic -lhdf5 -lmatheval

clean:
	rm -f vvcompose

install: all | $(PREFIX)/bin $(PREFIX)/share/vvhd/bash_completion.d
	cp vvcompose -t $(PREFIX)/bin/
	cp completion_vvcompose -t $(PREFIX)/share/vvhd/bash_completion.d
ifeq ($(SHELL),/bin/bash)
	bash -c "source completion_vvcompose"
endif

uninstall:
	rm -f $(PREFIX)/bin/vvcompose
	rm -f $(PREFIX)/share/vvhd

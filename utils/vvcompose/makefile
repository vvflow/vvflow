include ../../vvhd.mk

all: vvcompose

CXXFLAGS += -I../../headers
LDFLAGS += -L../../bin
vvcompose: vvcompose.cpp ../../bin/libvvhd.so
	$(CXX) $(CXXFLAGS) -std=c++0x $(LDFLAGS) -lvvhd $< -o ./$@

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
	rm -f $(PREFIX)/share/vvhd/bash_completion.d/completion_vvcompose

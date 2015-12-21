include ../../vvhd.mk

all: vvflow

CXXFLAGS += -I../../headers
LDFLAGS += -L../../bin
vvflow: vvflow.cpp  ../../bin/libvvhd.so
	$(CXX) $(CXXFLAGS) -std=c++0x $(LDFLAGS) -lvvhd $< -o ./$@

clean:
	rm -f vvflow

install: all | $(PREFIX)/bin
	cp vvflow -t $(PREFIX)/bin/

uninstall:
	rm -f $(PREFIX)/bin/vvflow



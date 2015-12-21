include ../../vvhd.mk

all: vvxtract

CXXFLAGS += -I../../headers
LDFLAGS += -L../../bin
vvxtract: vvxtract.cpp ../../bin/libvvhd.so
	$(CXX) $(CXXFLAGS) -std=c++0x $(LDFLAGS) -lvvhd $< -o ./$@

clean:
	rm -f vvxtract

install: all | $(PREFIX)/bin
	cp vvxtract -t $(PREFIX)/bin/
	cp h5xtract -t $(PREFIX)/bin/

uninstall:
	rm -f $(PREFIX)/bin/vvxtract
	rm -f $(PREFIX)/bin/h5xtract



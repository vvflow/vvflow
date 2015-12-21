include ../../vvhd.mk

all: libvvplot libvvplot.so

CXXFLAGS += -I../../headers
LDFLAGS += -L../../bin
libvvplot: source/libvvplot_main.cpp libvvplot.so ../../bin/libvvhd.so
	$(CXX) $(CXXFLAGS) -std=c++11 -Wl,-rpath,\$$ORIGIN -L. -lvvplot $(LDFLAGS) -lvvhd $(LDLIBS) $< -o ./$@

libvvplot.so: $(patsubst %,source/%.cpp, map_extract map_vorticity map_pressure map_streamfunction map_velocity isoline)
	$(CXX) $(CXXFLAGS) -shared -fPIC -std=c++11 -Wl,-soname,libvvplot.so $^ -o ./$@

clean:
	rm -f libvvplot
	rm -f libvvplot.so

install: all | $(PREFIX)/bin $(PREFIX)/share/vvhd/bash_completion.d
	cp vvplot -t $(PREFIX)/bin/
	cp libvvplot -t $(PREFIX)/bin/
	cp libvvplot.so -t $(PREFIX)/bin/
	cp argparse_vvplot.py -t $(PREFIX)/bin/
	cp completion_vvplot -t $(PREFIX)/share/vvhd/bash_completion.d/
ifeq ($(SHELL),/bin/bash)
	bash -c "source completion_vvplot"
endif

uninstall:
	rm -f $(PREFIX)/bin/vvplot
	rm -f $(PREFIX)/bin/libvvplot
	rm -f $(PREFIX)/bin/libvvplot.so
	rm -f $(PREFIX)/bin/argparse_vvplot.py
	rm -f $(PREFIX)/share/vvhd/bash_completion.d/completion_vvplot

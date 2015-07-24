PREFIX=$(HOME)/.local

ifeq ($(CXX),icpc)
        CXXFLAGS+= -O3 -Wall -openmp -mkl=parallel -debug all
	LDFLAGS =
else
        CXXFLAGS+= -O3 -Wall -fopenmp -g
	LDFLAGS = -llapack
endif

names:= vvdistrib
scripts:= vvplot argparse_vvplot.py
exes:= $(patsubst %,%_exe, $(names))

all: $(exes) libvvplot.so libvvplot

%_exe: %.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -std=c++0x -lhdf5 -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

libvvplot.so: $(patsubst %,source/%.cpp, map_extract map_vorticity map_pressure map_streamfunction map_velocity isoline)
	$(CXX) $(CXXFLAGS) $^ -o $@ -shared -fPIC -std=c++0x -Wl,-soname,libvvplot.so -Wl,-Bstatic -lvvhd -Wl,-Bdynamic -lhdf5 -lmatheval $(LDFLAGS)

libvvplot: source/libvvplot_main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -std=c++0x -L. -Wl,-rpath,\$$ORIGIN -lvvplot -Wl,-Bstatic -lvvhd -Wl,-Bdynamic -lhdf5 -lmatheval $(LDFLAGS)

clean:
	rm -f $(exes)
	rm -f libvvplot.so

install: | $(PREFIX)/bin
	cp $(scripts) $(exes) libvvplot.so libvvplot $(PREFIX)/bin
	bash -c "source completion_vvplot; _install_vvplot_completion"

$(PREFIX)/bin:
	mkdir -p $@

uninstall:
	rm -f $(patsubst  %,$(PREFIX)/bin/%, $(exes) $(scripts))
	bash -c "source completion_vvplot; _uninstall_vvplot_completion"


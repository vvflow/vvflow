PREFIX=$(HOME)/.local

ifeq ($(CXX),icpc)
        CXXFLAGS+= -O3 -Wall -openmp -mkl=parallel -debug all
else
        CXXFLAGS+= -O3 -Wall -fopenmp -g
endif

names:= vvdistrib
scripts:= vvplot argparse_vvplot.py
exes:= $(patsubst %,%_exe, $(names))

all: $(exes) libvvplot.so libvvplot

%_exe: %.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -std=c++0x -lhdf5 -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

libvvplot.so: $(patsubst %,source/%.cpp, map_extract map_vorticity map_pressure map_streamfunction isoline)
	$(CXX) $(CXXFLAGS) $^ -o $@ -shared -fPIC -std=c++0x -mkl -lhdf5 -Wl,-soname,libvvplot.so -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

libvvplot: source/libvvplot_main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -std=c++0x -lhdf5 -L. -Wl,-rpath,\$$ORIGIN -lvvplot -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

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


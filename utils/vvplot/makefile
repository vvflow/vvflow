CC = icc -O3 -openmp
PREFIX=$(HOME)/.local
names:= vvdistrib
scripts:= vvplot argparse_vvplot.py
exes:= $(patsubst %,%_exe, $(names))

all: $(exes) libvvplot.so libvvplot

%_exe: %.cpp
	$(CC) $^ -o $@ -g -std=c++0x -mkl=parallel -lhdf5 -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

libvvplot.so: $(patsubst %,source/%.cpp, map_extract map_vorticity map_pressure map_streamfunction isoline)
	icc $^ -o $@ -g -debug all -openmp -shared -fPIC -std=c++0x -mkl -lhdf5 -Wl,-soname,libvvplot.so -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

libvvplot: source/libvvplot_main.cpp
	icc $^ -o $@ -g -debug all -openmp -mkl -std=c++0x -lhdf5 -L. -Wl,-rpath,\$$ORIGIN -lvvplot -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

clean:
	rm -f $(exes)
	rm -f libvvplot.so

install: | $(PREFIX)/bin
	cp $(scripts) $(exes) $(PREFIX)/bin
	bash -c "source completion_vvplot; _install_vvplot_completion"

$(PREFIX)/bin:
	mkdir -p $@

uninstall:
	rm -f $(patsubst  %,$(PREFIX)/bin/%, $(exes) $(scripts))
	bash -c "source completion_vvplot; _uninstall_vvplot_completion"


CC = icc -O3 -openmp
PREFIX=$(HOME)/.local
names:= isoline streamlines heatplot vvdistrib
scripts:= vvplot argparse_vvplot.py
exes:= $(patsubst %,%_exe, $(names))

all: $(exes) libvvplot.so

vvmap: vvmap.cpp
	$(CC) $^ -o $@ -g -std=c++0x -mkl=parallel -lhdf5 -Wl,-Bstatic -lvvhd -Wl,-Bdynamic
%_exe: %.cpp
	$(CC) $^ -o $@ -g -std=c++0x -mkl=parallel -lhdf5 -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

libvvplot.so: $(patsubst %,source/%.cpp, map_extract map_vorticity map_pressure map_streamfunction isoline)
	icc $^ -o $@ -shared -fPIC -openmp -mkl -std=c++0x -Wl,-soname,libvvplot.so,-e,main -lhdf5 -Wl,-Bstatic -lvvhd -Wl,-Bdynamic -liomp5

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


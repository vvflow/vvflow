CC = icc -O3 -openmp
PREFIX=$(HOME)/.local
names:= presplot isoline streamlines heatplot vorticity vvdistrib vvmap
scripts:= vvplot argparse_vvplot.py
exes:= $(patsubst %,%_exe, $(names))

all: $(exes)

vvmap: vvmap.cpp
	$(CC) $^ -o $@ -g -std=c++0x -mkl=parallel -lhdf5 -Wl,-Bstatic -lvvhd -Wl,-Bdynamic
%_exe: %.cpp
	$(CC) $^ -o $@ -g -std=c++0x -mkl=parallel -lhdf5 -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

clean:
	rm -f $(exes)

install: | $(PREFIX)/bin
	cp $(scripts) $(exes) $(PREFIX)/bin
	bash -c "source completion_vvplot; _install_vvplot_completion"

$(PREFIX)/bin:
	mkdir -p $@

uninstall:
	rm -f $(patsubst  %,$(PREFIX)/bin/%, $(exes) $(scripts))
	bash -c "source completion_vvplot; _uninstall_vvplot_completion"


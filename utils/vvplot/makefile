CC = icc -O3 -openmp
PREFIX=$(HOME)/.local
names:= presplot isoline streamlines heatplot vorticity vvdistrib
scripts:= vvplot argparse_vvplot.py
exes:= $(patsubst %,%_exe, $(names))

all: $(exes)

%_exe: %.cpp
	$(CC) $^ -o $@ -std=c++0x -mkl=parallel -lvvhd

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


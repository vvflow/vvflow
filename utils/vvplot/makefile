CC = icc -O3 -openmp
names:= presplot isoline streamlines heatplot vorticity vvdistrib
scripts:= vvplot vvjoin argparse_vvplot.py
exes:= $(patsubst %,%_exe, $(names))

all: $(exes)

%_exe: %.cpp
	$(CC) $^ -o $@ $(VVHD) -std=c++0x -mkl=parallel

clean:
	rm -f $(exes)

install:
	cp $(scripts) $(exes) ~/.bin
	bash -c "source completion_vvplot; _install_vvplot_completion"

uninstall:
	rm -f $(patsubst  %,~/.bin/%, $(exes) $(scripts))
	bash -c "source completion_vvplot; _uninstall_vvplot_completion"


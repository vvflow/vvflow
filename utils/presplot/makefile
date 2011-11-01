CC = icc -O3 -mkl=parallel -openmp

all:
	$(CC) presplot.cpp -o presplot_exe $(VVHD) -std=c++0x

clean:
	rm -f presplot_exe

install:
	cp presplot_exe presplot ~/.bin

uninstall:
	rm -f ~/.bin/presplot_exe ~/.bin/presplot

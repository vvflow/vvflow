CC = icc -O3 -mkl=parallel -openmp

all:
	$(CC) streamlines.cpp -o streamlines_exe $(VVHD) -std=c++0x
	$(CC) isoline.cpp -o isoline_exe $(VVHD) -std=c++0x

clean:
	rm -f streamlines_exe isoline_exe

install:
	cp isoline_exe streamlines_exe streamlines ~/.bin

uninstall:
	rm -f ~/.bin/streamlines_exe ~/.bin/streamlines ~/.bin/isoline_exe

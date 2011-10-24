CC = icc -O3 -mkl=parallel -g -debug all

all:
	$(CC) presplot.cpp -o presplot $(VVHD) -std=c++0x

clean:
	rm -f presplot

install:
	cp presplot ~/.bin

uninstall:
	rm -f ~/.bin/presplot

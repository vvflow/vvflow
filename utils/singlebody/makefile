CC = icc -O3 -openmp

all:
	$(CC) singlebody.cpp -o vvhd_sb $(VVHD) -std=c++0x -mkl=parallel

clean:
	rm -f vvhd_sb

install:
	cp vvhd_sb ~/.bin/

uninstall:
	rm -f ~/.bin/vvhd_sb



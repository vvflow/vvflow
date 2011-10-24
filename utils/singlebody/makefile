CC = icc -O3 -openmp -mkl=parallel

all:
	$(CC) singlebody.cpp -o vvhd_sb $(VVHD) -std=c++0x

clean:
	rm -f vvhd_sb

install:
	cp vvhd_sb ~/.bin/

uninstall:
	rm -f ~/.bin/vvhd_sb



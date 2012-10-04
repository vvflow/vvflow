CC = icc -O3 -openmp -g -debug all

all:
	$(CC) simpleflow.cpp -o vvhd_sf $(VVHD) -std=c++0x -mkl=parallel
	$(CC) vvcompose.cpp -o vvcompose $(VVHD) -std=c++0x -mkl=parallel
	$(CC) vvxtract.cpp -o vvxtract $(VVHD) -std=c++0x -mkl=parallel

clean:
	rm -f vvhd_sf vvcompose

install:
	cp vvhd_sf vvcompose ~/.bin/

uninstall:
	rm -f ~/.bin/vvhd_sf ~/.bin/vvcompose



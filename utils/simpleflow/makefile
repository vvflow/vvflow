CC = icc -O3 -openmp

all:
	$(CC) simpleflow.cpp -o vvhd_sf $(VVHD) -std=c++0x -mkl=parallel

clean:
	rm -f vvhd_sf

install:
	cp vvhd_sf ~/.bin/

uninstall:
	rm -f ~/.bin/vvhd_sf



CC = icc -O3 -openmp -g -debug all

all:
	$(CC) simpleflow.cpp -o vvhd_sf $(VVHD) -std=c++0x -mkl=parallel
	$(CC) t_simpleflow.cpp -o t_vvhd_sf $(VVHD) -std=c++0x -mkl=parallel
	$(CC) vvcompose.cpp -o vvcompose $(VVHD) -std=c++0x -mkl=parallel
	$(CC) vvxtract.cpp -o vvxtract $(VVHD) -std=c++0x -mkl=parallel

clean:
	rm -f t_vvhd_sf vvhd_sf vvcompose vvxtract

install:
	cp t_vvhd_sf vvhd_sf vvcompose vvxtract calc.py ~/.bin/

uninstall:
	rm -f ~/.bin/t_vvhd_sf ~/.bin/vvhd_sf ~/.bin/vvcompose ~/.bin/vvxtract ~/.bin/calc.py



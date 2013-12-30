CC = icc -O3 -openmp -g -debug all

.PHONY: vvhd_sf t_vvhd_sf vvcompose vvxtract
all: vvhd_sf t_vvhd_sf vvcompose vvxtract

vvhd_sf: simpleflow.cpp
t_vvhd_sf: t_simpleflow.cpp
vvcompose: vvcompose.cpp
vvxtract: vvxtract.cpp

t_vvhd_sf vvhd_sf vvcompose vvxtract:
	$(CC) $^ -o $@ $(VVHD) -std=c++0x -mkl=parallel -lhdf5 -lhdf5_cpp

clean:
	rm -f t_vvhd_sf vvhd_sf vvcompose vvxtract

install:
	cp t_vvhd_sf vvhd_sf vvcompose vvxtract calc.py ~/.bin/

uninstall:
	rm -f ~/.bin/t_vvhd_sf ~/.bin/vvhd_sf ~/.bin/vvcompose ~/.bin/vvxtract ~/.bin/calc.py



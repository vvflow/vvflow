CC = icc -O3 -openmp -g -debug all
PREFIX = ~/.local

.PHONY: vvflow
all: vvflow

vvflow: vvflow.cpp sensors.cpp
	$(CC) $^ -o $@ $(VVHD) -std=c++0x -mkl=parallel -lhdf5 -lhdf5_cpp

clean:
	rm -f vvflow

install:
	cp vvflow calc.py ~/.bin/

uninstall:
	rm -f $(PREFIX)/bin/vvflow $(PREFIX)/bin/calc.py



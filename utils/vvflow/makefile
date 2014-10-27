CC = icc -O3 -openmp -g -debug all
PREFIX = ~/.local

.PHONY: vvflow
all: vvflow

vvflow: vvflow.cpp
	$(CC) $^ -o $@ -std=c++0x -mkl=parallel -lhdf5 -lmatheval -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

clean:
	rm -f vvflow

install: | $(PREFIX)/bin
	cp vvflow calc.py $(PREFIX)/bin/

$(PREFIX)/bin:
	mkdir -p $@

uninstall:
	rm -f $(PREFIX)/bin/vvflow $(PREFIX)/bin/calc.py



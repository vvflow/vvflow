CC = icc -O3 -openmp -g -debug all
PREFIX = ~/.local

.PHONY: vvcompose
all: vvcompose

vvcompose: vvcompose.cpp
	$(CC) $^ -o $@ -std=c++0x -mkl=parallel -lhdf5 -lmatheval -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

clean:
	rm -f vvcompose

install: | $(PREFIX)/bin
	cp vvcompose $(PREFIX)/bin/

$(PREFIX)/bin:
	mkdir -p $@

uninstall:
	rm -f $(PREFIX)/bin/vvcompose



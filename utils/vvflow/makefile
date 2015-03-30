PREFIX = ~/.local

ifeq ($(CXX),icpc)
        CXXFLAGS+= -debug all  -O3 -Wall -openmp -mkl=parallel
        AR = xiar
else
        CXXFLAGS+= -O3 -Wall -fopenmp -llapack
endif

all: vvflow

vvflow: vvflow.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -std=c++0x -lhdf5 -lmatheval -Wl,-Bstatic -lvvhd -Wl,-Bdynamic

clean:
	rm -f vvflow

install: | $(PREFIX)/bin
	cp vvflow calc.py $(PREFIX)/bin/

$(PREFIX)/bin:
	mkdir -p $@

uninstall:
	rm -f $(PREFIX)/bin/vvflow $(PREFIX)/bin/calc.py



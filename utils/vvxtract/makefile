PREFIX = ~/.local

ifeq ($(CXX),icpc)
        CXXFLAGS+= -O3 -Wall -openmp -mkl=parallel
        AR = xiar
else
        CXXFLAGS+= -O3 -Wall -fopenmp
endif

all: vvxtract

vvxtract: vvxtract.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -std=c++0x -Wl,-Bstatic -lvvhd -Wl,-Bdynamic -lhdf5 -lmatheval

clean:
	rm -f vvxtract

install: | $(PREFIX)/bin
	cp vvxtract h5xtract $(PREFIX)/bin/

$(PREFIX)/bin:
	mkdir -p $@

uninstall:
	rm -f $(PREFIX)/bin/vvxtract



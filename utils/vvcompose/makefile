PREFIX = ~/.local

ifeq ($(CXX),icpc)
        CXXFLAGS+= -debug all  -O3 -Wall -openmp -mkl=parallel
        AR = xiar
else
        CXXFLAGS+= -O3 -Wall -fopenmp
endif

all: vvcompose

vvcompose: vvcompose.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -std=c++0x -Wl,-Bstatic -lvvhd -Wl,-Bdynamic -lhdf5 -lmatheval

clean:
	rm -f vvcompose

install: | $(PREFIX)/bin
	cp vvcompose $(PREFIX)/bin/
	bash -c "source completion_vvcompose; _install_vvcompose_completion"

$(PREFIX)/bin:
	mkdir -p $@

uninstall:
	rm -f $(PREFIX)/bin/vvcompose
	bash -c "source completion_vvcompose; _uninstall_vvcompose_completion"



PREFIX = $(HOME)/.local
LDLIBS := -lhdf5 -lmatheval
CXXFLAGS += -std=c++11

ifeq ($(CXX),icpc)
	CXXFLAGS += -g -debug all -O3 -Wall -openmp -mkl
	LDFLAGS += -mkl=parallel
	AR = xiar
else
	CXXFLAGS += -O3 -g -Wall -fopenmp
	LDLIBS += -llapack
endif

$(PREFIX)/%:
	mkdir $@ -p

.PHONY: all clean install uninstall

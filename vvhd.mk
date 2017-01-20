PREFIX           = $(HOME)/.local
LDLIBS          :=
LDLIBS          += $(shell pkg-config --silence-errors --libs hdf5-serial || echo -lhdf5)
LDLIBS          += $(shell pkg-config --silence-errors --libs matheval || echo -lmatheval)
CXXFLAGS        += -std=c++11
CXXFLAGS        += $(shell pkg-config --silence-errors --cflags hdf5-serial)

ifeq ($(CXX),icpc)
	CXXFLAGS += -g -debug all -O3 -Wall -openmp -mkl
	LDFLAGS += -mkl=parallel
	AR = xiar
else
	CXXFLAGS += -O3 -g -Wall -fopenmp
	CXXFLAGS += $(shell pkg-config --silence-errors --cflags lapack)
	LDLIBS += $(shell pkg-config --silence-errors --libs cblas)
	LDLIBS += $(shell pkg-config --silence-errors --libs lapack || echo -llapack)
endif

$(PREFIX)/%:
	mkdir $@ -p

define \n


endef

.PHONY: all clean install uninstall

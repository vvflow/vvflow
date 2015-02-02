#------------------------------------------------#
#                 VVHD makefile                  #
#       to understand what's written here        #
#  read http://www.opennet.ru/docs/RUS/gnumake/  #
#                                                #
#  (c) Rosik                                     #
#------------------------------------------------#

#
# Usage:
# make [optimization="-O3 -pg"] [warnings="-Wall"]
# make [INSTALLDIR="~/.libVVHDinstall"] install
#

#------------------------------------------------#
#                   VARIABLES                    #
#------------------------------------------------#

ifeq ($(CXX),icpc)
        CXXFLAGS+= -g -debug all -O3 -xSSE4.2 -Wall -openmp -mkl
        AR = xiar
else
        CXXFLAGS+= -O3 -Wall -fopenmp
endif

parts 	:= core modules
core_objects 	:= space body shellscript sorted_tree stepdata
modules_objects := flowmove epsfast diffusivefast matrix convectivefast
VPATH := $(addprefix source/, $(parts) ) 
# VPATH is special make var 

INCLUDE 		:= headers/

INSTALLDIR 		:= $(HOME)/.local
GITINFO         := -DDEF_GITINFO="\"$(shell git log -1 | head -n1 | cut -d" " -f2)\""
GITDIFF 		:= -DDEF_GITDIFF="\"$(shell git diff --name-only)\""

#------------------------------------------------#
#                    TARGETS                     #
#------------------------------------------------#


all: bin/libvvhd.a bin/libvvhd.so

clean:
	rm -rf bin

install: | $(INSTALLDIR)/lib/ $(INSTALLDIR)/include/
	cp bin/libvvhd.a bin/libvvhd.so -t $(INSTALLDIR)/lib/
	cp headers/*.h -t $(INSTALLDIR)/include/

uninstall:
	rm -f $(patsubst %, $(INSTALLDIR)/lib/libvvhd.%, a so)
	rm -f $(patsubst headers/%, $(INSTALLDIR)/include/%, $(wildcard headers/*.h))
	rmdir $(INSTALLDIR)/lib/ $(INSTALLDIR)/include/ $(INSTALLDIR)/ --ignore-fail-on-non-empty

#------------------------------------------------#
#                     RULES                      #
#------------------------------------------------#

bin/%.o: %.cpp headers/%.h headers/elementary.h | bin/
	$(CXX) $(CXXFLAGS) $< -o $@ $(addprefix -I, $(INCLUDE)) -c -std=c++0x -fPIC

bin/space.o: space.cpp headers/space.h headers/elementary.h | bin/
	$(CXX) $(CXXFLAGS) $< -o $@ \
	$(addprefix -I, $(INCLUDE)) -c -std=c++0x -fPIC \
	$(GITINFO) \
	$(GITDIFF)

bin/libvvhd.a: $(patsubst %, bin/%.o, $(core_objects) $(modules_objects))
	$(AR) ruc $@ $^
	ranlib $@

bin/libvvhd.so: $(patsubst %, bin/%.o, $(core_objects) $(modules_objects))
	$(CXX) -shared -fPIC -Wl,-soname,libvvhd.so -o $@ $^ -mkl=parallel

bin/:
	mkdir $@ -p

$(INSTALLDIR)/%:
	mkdir $@ -p


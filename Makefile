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
# make [PREFIX="~/.libVVHDinstall"] install
#

#------------------------------------------------#
#                   VARIABLES                    #
#------------------------------------------------#

include vvhd.mk

VPATH           := $(addprefix source/, core modules)
core_objects 	:= space body shellscript sorted_tree stepdata
modules_objects := flowmove epsfast diffusivefast matrix convectivefast
# VPATH is special make var 

CPATH           := headers/
GITINFO         := -DDEF_GITINFO="\"$(shell git log -1 | head -n1 | cut -d" " -f2)\""
GITDIFF         := -DDEF_GITDIFF="\"$(shell git diff --name-only)\""

#------------------------------------------------#
#                    TARGETS                     #
#------------------------------------------------#


all: bin/libvvhd.a bin/libvvhd.so

clean:
	rm -rf bin

install: all | $(PREFIX)/lib/ $(PREFIX)/include/
	cp ./bin/libvvhd.a -t $(PREFIX)/lib/
	cp ./bin/libvvhd.so -t $(PREFIX)/lib/
	cp ./headers/*.h -t $(PREFIX)/include/

uninstall:
	rm -f $(patsubst %, $(PREFIX)/lib/libvvhd.%, a so)
	rm -f $(patsubst headers/%, $(PREFIX)/include/%, $(wildcard headers/*.h))
	rmdir $(PREFIX)/lib/ $(PREFIX)/include/ $(PREFIX)/ --ignore-fail-on-non-empty

.PHONY: all clean install uninstall
#------------------------------------------------#
#                     RULES                      #
#------------------------------------------------#

bin/%.o: %.cpp $(wildcard headers/*.h) | bin/
	$(CXX) $(CXXFLAGS) -std=c++11 -fPIC -c ./$< -o $@

bin/space.o: space.cpp headers/space.h headers/elementary.h | bin/
	$(CXX) $(CXXFLAGS) -std=c++11 -fPIC -c ./$< -o $@ \
	$(GITINFO) \
	$(GITDIFF)

bin/libvvhd.a: $(patsubst %, bin/%.o, $(core_objects) $(modules_objects))
	$(AR) Drc $@ $^
	ranlib $@

bin/libvvhd.so: $(patsubst %, bin/%.o, $(core_objects) $(modules_objects))
	$(CXX) $(LDFLAGS) -shared -fPIC $(LDLIBS) -Wl,-soname,libvvhd.so -o $@ $^

bin/:
	mkdir $@ -p

$(PREFIX)/%:
	mkdir $@ -p


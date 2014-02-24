#‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾#
#                 VVHD makefile                  #
#       to understand what's written here        #
#  read http://www.opennet.ru/docs/RUS/gnumake/  #
#                                                #
#  (c) Rosik                                     #
#________________________________________________#

#
# Usage:
# make [optimization="-O3 -pg"] [warnings="-Wall"]
# make [INSTALLDIR="~/.libVVHDinstall"] install
#

#‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾#
#                   VARIABLES                    #
#________________________________________________#

CC		= icc -O3 -g -openmp
AR		= xiar

parts 	:= core modules
core_objects 	:= space body tree shellscript sorted_tree
modules_objects := flowmove epsfast diffusivefast convective convectivefast matrix
VPATH := $(addprefix source/, $(parts) ) 
# VPATH is special make var 

warnings 		:= -Wall
INCLUDE 		:= headers/

INSTALLDIR 		:= $(HOME)/.local
GITINFO         := -DDEF_GITINFO="\"$(shell git log -1 | head -n1 | cut -d" " -f2)\""
GITDIFF 		:= -DDEF_GITDIFF="\"$(shell git diff --name-only)\""

#‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾#
#                    TARGETS                     #
#________________________________________________#


all: bin/libvvhd.a bin/libvvhd.so

clean:
	rm -rf bin

install: | $(INSTALLDIR)/lib/ $(INSTALLDIR)/include/
	cp bin/libvvhd.a bin/libvvhd.so -t $(INSTALLDIR)/lib/
	cp headers/*.h -t $(INSTALLDIR)/include/

uninstall:
	rm -f $(patsubst %, $(INSTALLDIR)/lib/libVVHD%.a, $(parts))
	rm -f $(patsubst headers/%, $(INSTALLDIR)/include/%, $(wildcard headers/*.h))
	rmdir $(INSTALLDIR)/lib/ $(INSTALLDIR)/include/ $(INSTALLDIR)/ --ignore-fail-on-non-empty

#‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾#
#                     RULES                      #
#________________________________________________#

bin/%.o: %.cpp headers/%.h headers/elementary.h | bin/
	$(CC) $< -o $@ \
	$(optimization) $(warnings) $(addprefix -I, $(INCLUDE)) -c -std=c++0x -fPIC

bin/space.o: space.cpp headers/space.h headers/elementary.h | bin/
	$(CC) $< -o $@ \
	$(optimization) $(warnings) $(addprefix -I, $(INCLUDE)) -c -std=c++0x -fPIC \
	$(GITINFO) \
	$(GITDIFF)

bin/libvvhd.a: $(patsubst %, bin/%.o, $(core_objects) $(modules_objects))
	$(AR) ruc $@ $^
	ranlib $@

bin/libvvhd.so: $(patsubst %, bin/%.o, $(core_objects))
	icc -shared -Wl,-soname,libvvhd.so -o $@ $^ -openmp -mkl=parallel

bin/:
	mkdir $@ -p

$(INSTALLDIR)/%:
	mkdir $@ -p

export_vars:
	export VVHD="-I $(INSTALLDIR)/include -L $(INSTALLDIR)/lib -lVVHDmodules -lVVHDcore"


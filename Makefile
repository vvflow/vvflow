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
core_objects 	:= space body tree
modules_objects := flowmove flowmove2 epsfast diffusivefast convective convectivefast
VPATH := $(addprefix source/, $(parts) ) 
# VPATH is special make var 

warnings 		:= -Wall
INCLUDE 		:= headers/

INSTALLDIR 		:= ~/.libVVHDinstall

#‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾#
#                    TARGETS                     #
#________________________________________________#


all: $(patsubst %, bin/libVVHD%.a, $(parts))

clean:
	rm -rf bin

install: | $(INSTALLDIR)/lib/ $(INSTALLDIR)/include/
	cp $(patsubst %, bin/libVVHD%.a, $(parts)) -t $(INSTALLDIR)/lib/
	cp headers/*.h -t $(INSTALLDIR)/include/

uninstall:
	rm -f $(patsubst %, $(INSTALLDIR)/lib/libVVHD%.a, $(parts))
	rm -f $(patsubst headers/%, $(INSTALLDIR)/include/%, $(wildcard headers/*.h))
	rmdir $(INSTALLDIR)/lib/ $(INSTALLDIR)/include/ $(INSTALLDIR)/ --ignore-fail-on-non-empty

#‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾#
#                     RULES                      #
#________________________________________________#

bin/%.o: %.cpp headers/%.h headers/elementary.h | bin/
	$(CC) $< $(optimization) $(warnings) $(addprefix -I, $(INCLUDE)) -c -o $@ -std=c++0x

bin/libVVHDcore.a: $(patsubst %, bin/%.o, $(core_objects))
bin/libVVHDmodules.a: $(patsubst %, bin/%.o, $(modules_objects))
bin/libVVHD%.a:
	$(AR) ruc $@ $^
	ranlib $@

bin/:
	mkdir $@ -p

$(INSTALLDIR)/%:
	mkdir $@ -p

export_vars:
	export VVHD="-I $(INSTALLDIR)/include -L $(INSTALLDIR)/lib -lVVHDmodules -lVVHDcore"


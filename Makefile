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

core_objects    := space body shellscript sorted_tree stepdata
modules_objects := flowmove epsfast diffusivefast matrix convectivefast

CPATH           := headers/:$(CPATH)
LIBRARY_PATH    := bin/:$(CPATH)
GITINFO         := -DDEF_GITINFO="\"$(shell git log -1 | head -n1 | cut -d" " -f2)\""
GITDIFF         := -DDEF_GITDIFF="\"$(shell git diff --name-only)\""
export CPATH
export LIBRARY_PATH

#------------------------------------------------#
#                    TARGETS                     #
#------------------------------------------------#

# DEPENDENCIES
VPATH           += $(addprefix utils/, vvcompose vvxtract vvflow vvplot scripts)
include utils/vvcompose/make.mk
include utils/vvxtract/make.mk
include utils/vvflow/make.mk
include utils/vvplot/make.mk
include utils/scripts/make.mk
# ---

all: bin/libvvhd.a bin/libvvhd.so $(TARGETS_ALL)

clean:
	rm -rf bin

install: libvvhd_install $(TARGETS_INSTALL) | $(PREFIX)/lib/
libvvhd_install:
	cp ./bin/libvvhd.so -t $(PREFIX)/lib/

.PHONY: devinstall
devinstall: | $(PREFIX)/lib/ $(PREFIX)/include/
	cp ./bin/libvvhd.a -t $(PREFIX)/lib/
	cp ./headers/*.h -t $(PREFIX)/include/


uninstall: libvvhd_uninstall $(TARGETS_UNINSTALL)
	rm -rf $(PREFIX)/share/vvhd/
libvvhd_uninstall:
	rm -f $(PREFIX)/lib/libvvhd.a
	rm -f $(PREFIX)/lib/libvvhd.so
	rm -f $(patsubst headers/%, $(PREFIX)/include/%, $(wildcard headers/*.h))
	rmdir --ignore-fail-on-non-empty $(PREFIX)/lib/ || true
	rmdir --ignore-fail-on-non-empty $(PREFIX)/include/ || true

#------------------------------------------------#
#                     RULES                      #
#------------------------------------------------#

bin/:
	@mkdir -p $@

vpath %.cpp source/core:source/modules
bin/%.o: %.cpp $(wildcard headers/*.h) | bin/
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o ./$@

bin/space.o: space.cpp headers/space.h headers/elementary.h | bin/
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o ./$@ \
	$(GITINFO) \
	$(GITDIFF)

bin/libvvhd.a: $(patsubst %, bin/%.o, $(core_objects) $(modules_objects))
	$(AR) Drc ./$@ $^
	ranlib ./$@

bin/libvvhd.so: $(patsubst %, bin/%.o, $(core_objects) $(modules_objects))
	$(CXX) $(LDFLAGS) -shared -fPIC $(LDLIBS) -Wl,-soname,libvvhd.so $^ -o ./$@

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

make = \
	echo "-- make $1 -C ./$2"; \
	$(MAKE) --no-print-directory $1 -C $2;

make_utils = \
	@$(call make,$@,utils/vvcompose) \
	$(call make,$@,utils/vvflow) \
	$(call make,$@,utils/vvplot) \
	$(call make,$@,utils/vvxtract)

all: bin/libvvhd.a bin/libvvhd.so
	@$(call make_utils,$@)

clean:
	@$(call make_utils,$@)
	rm -rf bin

install: all | $(PREFIX)/lib/
	cp ./bin/libvvhd.so -t $(PREFIX)/lib/
	@$(call make_utils,$@)

uninstall:
	@$(call make_utils,$@)
	rm -f $(PREFIX)/lib/libvvhd.so
	rmdir --ignore-fail-on-non-empty $(PREFIX)/lib/ || true

#------------------------------------------------#
#                     RULES                      #
#------------------------------------------------#

bin/:
	@mkdir -p $@

bin/%.o: %.cpp $(wildcard headers/*.h) | bin/
	$(CXX) $(CXXFLAGS) -std=c++11 -fPIC -c $< -o ./$@

bin/space.o: space.cpp headers/space.h headers/elementary.h | bin/
	$(CXX) $(CXXFLAGS) -std=c++11 -fPIC -c $< -o ./$@ \
	$(GITINFO) \
	$(GITDIFF)

bin/libvvhd.a: $(patsubst %, bin/%.o, $(core_objects) $(modules_objects))
	$(AR) Drc ./$@ $^
	ranlib $@

bin/libvvhd.so: $(patsubst %, bin/%.o, $(core_objects) $(modules_objects))
	$(CXX) $(LDFLAGS) -shared -fPIC $(LDLIBS) -Wl,-soname,libvvhd.so $^ -o ./$@

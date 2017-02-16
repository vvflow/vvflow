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
LIBRARY_PATH    := bin/:$(LIBRARY_PATH)
export CPATH
export LIBRARY_PATH

#------------------------------------------------#
#                    TARGETS                     #
#------------------------------------------------#

all: bin/libvvhd.a bin/libvvhd.so

# DEPENDENCIES
VPATH           += $(addprefix utils/, vvcompose vvxtract vvflow vvplot scripts)
include utils/vvcompose/make.mk
include utils/vvxtract/make.mk
include utils/vvflow/make.mk
include utils/vvplot/make.mk
include utils/scripts/make.mk
all: $(TARGETS_ALL)
# ---

clean:
	rm -rf ./bin

install: libvvhd_install $(TARGETS_INSTALL) | $(PREFIX)/share/vvhd/
	echo 'for d in $(PREFIX)/share/vvhd/bash_completion.d/*; do . $$d; done' \
	> $(PREFIX)/share/vvhd/bash_completion
libvvhd_install: | $(PREFIX)/lib/
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

test:
	@mkdir -p test_results
	@PATH=bin/:$$PATH ./tests/nobody/sink.sh $(PWD)/test_results
	@PATH=bin/:$$PATH ./tests/pipe/slip.sh $(PWD)/test_results
	@PATH=bin/:$$PATH ./tests/pipe/noslip.sh $(PWD)/test_results
	@PATH=bin/:$$PATH ./tests/pipe/semislip.sh $(PWD)/test_results
.PHONY: run_tests


#------------------------------------------------#
#                     RULES                      #
#------------------------------------------------#

bin/:
	@mkdir -p $@

vpath %.cpp source/core:source/modules
bin/%.o: %.cpp $(wildcard headers/*.h) | bin/
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o ./$@

bin/libvvhd.a: $(patsubst %, bin/%.o, gitinfo $(core_objects) $(modules_objects))
	$(AR) Drc ./$@ $^
	ranlib ./$@

bin/libvvhd.so: $(patsubst %, bin/%.o, gitinfo $(core_objects) $(modules_objects))
	$(CXX) $(LDFLAGS) -shared -fPIC $^ -Wl,-soname,libvvhd.so $(LDLIBS) -o ./$@

.PHONY: gitinfo.cpp
gitinfo.cpp:
	echo "const char* gitrev = \"$(shell git rev-parse HEAD)\";" > $@
	echo "const char* gitinfo = \"$(shell git describe --tags --always)\";" >> $@
	echo "const char* gitdiff = \"$(shell git diff --name-only)\";" >> $@

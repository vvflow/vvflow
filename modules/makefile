LIBNAME 		= VVHDmodules
LIBFILENAME 	= lib$(LIBNAME).a

CPP 			= gcc
CXX 			= g++
FORT 			= gfortran
INCLUDE 		= -I headers/

COBJECTS 		= convectivefast.o flowmove.o diffmergefast.o merge.o
COBJFLAGS 		= -c -g -Wall -O0
FOBJECTS 		= objectinfluence.o
FOBJFLAGS 		= -c -Wall -O3

all: incbuild bin/ $(LIBFILENAME) 
	./.incversion buildok

incbuild:
	./.incversion build

$(LIBFILENAME): $(COBJECTS) $(FOBJECTS)
	cd bin/ && ar ruc $(LIBFILENAME) $^
	ranlib bin/$@

%.o: source/%.cpp
	$(CXX) $< $(COBJFLAGS) -o bin/$@ $(INCLUDE) 

%.o: source/%.c
	$(CPP) $< $(COBJFLAGS) -o bin/$@ $(INCLUDE)

%.o: source/%.f90
	$(FORT) $< $(FOBJFLAGS) -o bin/$@ $(INCLUDE)

bin/:
	mkdir bin/

clean:
	rm -rf bin

install: /usr/local/lib/ /usr/local/include/libVVHD/
	cp bin/$(LIBFILENAME) -t /usr/local/lib/
	chmod 644 /usr/local/lib/$(LIBFILENAME)
	cp headers/* -t /usr/local/include/libVVHD/
	chmod 644 /usr/local/include/libVVHD/*

/usr/local/lib/:
	mkdir $@ -p
	chmod 755 $@

/usr/local/include/libVVHD/:
	mkdir $@ -p
	chmod 755 $@

uninstall:
	rm -f /usr/local/lib/$(LIBFILENAME)
	for head in `ls headers/`; do rm -f /usr/local/include/libVVHD/$$head; done;
	rmdir /usr/local/include/libVVHD --ignore-fail-on-non-empty

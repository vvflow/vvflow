LIBNAME 		= VVHDcore
LIBFILENAME 	= lib$(LIBNAME).a

CPP 			= gcc
CXX 			= g++
INCLUDE 		= -I headers/

COBJECTS 		= elementary.o space.o tree.o fexp.o body.o
COBJFLAGS 		= -c -g -Wall -O2
FOBJECTS 		= 

all: incbuild bin/ $(LIBFILENAME)
	./.incversion buildok

incbuild:
	./.incversion build

$(LIBFILENAME): $(COBJECTS) 
	cd bin/ && ar ruc $(LIBFILENAME) $^
	ranlib bin/$@

%.o: source/%.cpp
	$(CXX) $< $(COBJFLAGS) -o bin/$@ $(INCLUDE) 

%.o: source/%.c
	$(CPP) $< $(COBJFLAGS) -o bin/$@ $(INCLUDE)

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

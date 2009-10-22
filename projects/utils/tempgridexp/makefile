all: tempgridexp
	g++ tempgridexp.cpp -o tempgridexp -lVVHDcore -lVVHDmodules

install: tempgridexp
	cp tempgridexp /usr/local/bin

uninstall:
	rm -f /usr/local/bin/tempgridexp

tempgridexp:
	g++ tempgridexp.cpp -o tempgridexp -lVVHDcore -lVVHDmodules

clean:
	rm -f tempgridexp

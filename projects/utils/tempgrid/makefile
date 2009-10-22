all: tempgrid
	g++ tempgrid.cpp -o tempgrid -lVVHDcore -lVVHDmodules

install: tempgrid
	cp tempgrid /usr/local/bin

uninstall:
	rm -f /usr/local/bin/tempgrid

tempgrid:
	g++ tempgrid.cpp -o tempgrid -lVVHDcore -lVVHDmodules

clean:
	rm -f tempgrid

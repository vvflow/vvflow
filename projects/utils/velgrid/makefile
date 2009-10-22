all: velgrid
	g++ velgrid.cpp -o velgrid -lVVHDcore -lVVHDmodules

install: velgrid
	cp velgrid /usr/local/bin

uninstall:
	rm -f /usr/local/bin/velgrid

velgrid:
	g++ velgrid.cpp -o velgrid -lVVHDcore -lVVHDmodules

clean:
	rm -f velgrid

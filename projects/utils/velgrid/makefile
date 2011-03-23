all: velgrid
	g++ velgrid.cpp -o velgrid -lVVHDmodules -lVVHDcore

install: velgrid
	cp velgrid /usr/local/bin

uninstall:
	rm -f /usr/local/bin/velgrid

velgrid:
	g++ velgrid.cpp -o velgrid -lVVHDmodules -lVVHDcore

clean:
	rm -f velgrid

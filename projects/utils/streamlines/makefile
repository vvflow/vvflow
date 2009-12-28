all: streamlines
	g++ streamlines.cpp -o streamlines -lVVHDcore -lVVHDmodules

install: streamlines
	cp streamlines /usr/local/bin

uninstall:
	rm -f /usr/local/bin/streamlines

streamlines:
	g++ streamlines.cpp -o streamlines -lVVHDcore -lVVHDmodules

clean:
	rm -f streamlines

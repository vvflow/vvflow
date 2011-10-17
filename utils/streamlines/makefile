all:
	icc streamlines.cpp -o streamlines $(VVHD) -O3

clean:
	rm -f streamlines

install: streamlines
	cp streamlines /usr/local/bin

uninstall:
	rm -f /usr/local/bin/streamlines

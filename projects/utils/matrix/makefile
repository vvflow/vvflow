all:
	g++ genmatrix.cpp -o genmatrix -lVVHDmodules -lVVHDcore

clean:
	rm -f genmatrix

install:
	cp genmatrix invmatrix /usr/local/bin

uninstall:
	rm -f /usr/local/bin/genmatrix /usr/local/bin/invmatrix

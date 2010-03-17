all:
	g++ vvhdxtract.cpp -o vvhdxtract -lVVHDmodules -lVVHDcore -O3 `xml2-config --cflags --libs`

clean:
	rm -f vvhdxtract

install:
	cp vvhdxtract /usr/local/bin/vvhdxtract

uninstall:
	rm -f /usr/local/bin/vvhdxtract

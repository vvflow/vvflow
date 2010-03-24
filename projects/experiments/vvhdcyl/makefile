all:
	g++ vvhdcyl.cpp -o vvhdcyl -lVVHDmodules -lVVHDcore -lpthread -O3 `xml2-config --cflags --libs`

clean:
	rm -f vvhdcyl

install:
	cp vvhdcyl /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/vvhdcyl

PREFIX = ~/.local

.PHONY: vvxtract
all: vvxtract

vvxtract: vvxtract.cpp
	$(CXX) $^ -o $@ -std=c++0x -lvvhd -lhdf5 -lmatheval

clean:
	rm -f vvxtract

install: | $(PREFIX)/bin
	cp vvxtract $(PREFIX)/bin/

$(PREFIX)/bin:
	mkdir -p $@

uninstall:
	rm -f $(PREFIX)/bin/vvxtract



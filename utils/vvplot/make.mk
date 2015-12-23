TARGETS_ALL       += bin/libvvplot bin/libvvplot.so
TARGETS_INSTALL   += vvplot_install vvplot_completion_install
TARGETS_UNINSTALL += vvplot_uninstall

bin/libvvplot: source/libvvplot_main.cpp bin/libvvhd.so bin/libvvplot.so
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -Wl,-rpath,\$$ORIGIN -lvvplot -lvvhd $(LDLIBS) $< -o ./$@

bin/libvvplot.so: $(patsubst %,source/%.cpp, map_extract map_vorticity map_pressure map_streamfunction map_velocity isoline)
	$(CXX) $(CXXFLAGS) -shared -fPIC -Wl,-soname,libvvplot.so $^ -o ./$@

vvplot_install: vvplot argparse_vvplot.py | $(PREFIX)/bin
	cp ./bin/libvvplot -t $(PREFIX)/bin/
	cp ./bin/libvvplot.so -t $(PREFIX)/bin/
	cp $^ -t $(PREFIX)/bin/
vvplot_completion_install: completion_vvplot | $(PREFIX)/share/vvhd/bash_completion.d
	cp $^ -t $(PREFIX)/share/vvhd/bash_completion.d/

vvplot_uninstall:
	rm -f $(PREFIX)/bin/vvplot
	rm -f $(PREFIX)/bin/libvvplot
	rm -f $(PREFIX)/bin/libvvplot.so
	rm -f $(PREFIX)/bin/argparse_vvplot.py
	rm -f $(PREFIX)/share/vvhd/bash_completion.d/completion_vvplot

TARGETS_ALL       += bin/libvvplot bin/libvvplot.so
TARGETS_INSTALL   += vvplot_install vvplot_completion_install
TARGETS_UNINSTALL += vvplot_uninstall

bin/libvvplot: bin/libvvhd.so bin/libvvplot.so
bin/libvvplot: bin/libvvplot_main.o
	$(CXX) $(CXXFLAGS) $^ -Wl,-rpath,\$$ORIGIN -Wl,-rpath-link,bin $(LDFLAGS) -lvvplot -lvvhd -lhdf5 -o ./$@

bin/libvvplot.so: bin/libvvhd.so
bin/libvvplot.so: $(patsubst %,bin/%.o, map_extract map_vorticity map_pressure map_streamfunction map_velocity isoline)
	$(CXX) $(CXXFLAGS) -shared -fPIC $^ -Wl,-soname,libvvplot.so -Wl,-rpath,\$$ORIGIN -lvvhd $(LDLIBS) -o ./$@

bin/%.o: source/%.cpp $(wildcard headers/*.h) | bin/
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o ./$@

vvplot_install: vvplot argparse_vvplot.py | $(PREFIX)/bin
	$(foreach f,bin/libvvplot bin/libvvplot.so $^,\
		cp $(f) -t $(PREFIX)/bin${\n}\
	)
vvplot_completion_install: completion_vvplot | $(PREFIX)/share/vvhd/bash_completion.d
	cp $^ -t $(PREFIX)/share/vvhd/bash_completion.d/

vvplot_uninstall:
	$(foreach f,vvplot libvvplot libvvplot.so argparse_vvplot.py,\
		rm -f $(PREFIX)/bin/$(f)${\n}\
	)
	rm -f $(PREFIX)/share/vvhd/bash_completion.d/completion_vvplot

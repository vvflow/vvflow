.PHONY: all compose run plot clean

all: compose run plot

compose: re600_n350.h5
re600_n350.h5:
	vvcompose cylinder.lua

run: stepdata_re600_n350.h5
stepdata_re600_n350.h5:
	@ echo Running CFD simulation up to t=20
	@ echo It may take about a minute or two
	vvflow --progress ./re600_n350.h5

plot:

clean:
	rm -rf results_re600_n350
	rm -rf stepdata_re600_n350.h5
	rm -rf *.png


.PHONY: all compose run plot clean

all: compose run plot

compose: re600_n350.h5
re600_n350.h5:
	vvcompose cylinder.lua

run: stepdata_re600_n350.h5
stepdata_re600_n350.h5:
	@ echo "--" Running CFD simulation up to t=20
	@ echo "--" It may take about a minute or two
	vvflow --progress ./re600_n350.h5

plot: bvs.png bvg.png bvp.png cx.png cy.png
bvs.png: results_re600_n350/000400.h5
	vvplot $< $@ -x -2,18 -BVS
bvg.png: results_re600_n350/000400.h5
	vvplot $< $@ -x -2,18 -B --V 0 --G 4
bvp.png: results_re600_n350/000400.h5
	vvplot $< $@ -x -2,18 -B --V 0 -P --res-hi 256
cx.png: stepdata_re600_n350.h5
	vvxtract $< time body00/force_holder | gpquick -p -u 1:2 -y 0 1 > $@
cy.png: stepdata_re600_n350.h5
	vvxtract $< time body00/force_holder | gpquick -p -u 1:3 -y -1 1 > $@

clean:
	rm -rf results_re600_n350
	rm -rf *.png
	rm -rf *.h5


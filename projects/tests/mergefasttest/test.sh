#!/bin/bash

make && ./exe

vvhdplot --vorticity 1.vort -2 6
vvhdplot --vorticity 2.vort -2 6
vvhdplot --vorticity 3.vort -2 6

streamlines 1.vort -2 6 -3.2 3.2 0.2 0.2 > /dev/null
streamlines 2.vort -2 6 -3.2 3.2 0.2 0.2 > /dev/null
streamlines 3.vort -2 6 -3.2 3.2 0.2 0.2 > /dev/null

vvhdplot --streamlines 1.vort.streamlines -2 6
vvhdplot --streamlines 2.vort.streamlines -2 6
vvhdplot --streamlines 3.vort.streamlines -2 6

#!/bin/bash

make && ./exe

vvhdplot --vorticity input -2 7
vvhdplot --vorticity output -2 7

streamlines input -2 7 -5 5 0.1 0.1 > /dev/null
streamlines output -2 7 -5 5 0.1 0.1 > /dev/null

defxmin=-2
defxmax=7
ymax=`echo "scale=5; (($defxmax) - ($defxmin)) *0.375" | bc`

echo "unset key
set xrange [$defxmin: $defxmax]
set yrange [-$ymax: $ymax]
set size ratio -1

set terminal png truecolor enhanced size 1024, 768
set output 'input.streamlines.png'

plot 'input.streamlines' using 1:2 with lines" | gnuplot

echo "unset key
set xrange [$defxmin: $defxmax]
set yrange [-$ymax: $ymax]
set size ratio -1

set terminal png truecolor enhanced size 1024, 768
set output 'output.streamlines.png'

plot 'output.streamlines' using 1:2 with lines" | gnuplot

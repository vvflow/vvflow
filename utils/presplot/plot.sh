#!/bin/bash


## Usage: ./plot.sh vort body map
time=$(cat ${1%v}i | tail -n2 | head -n1 | cut -d" " -f1)

echo "
unset key
unset colorbox
set xrange [-0.1: 1.4]
set yrange [-0.5: 0.5]
set cbrange [ -20 : 20 ]

set terminal jpeg truecolor enhanced size 900, 600 
set output '$1.jpg'
set lmargin screen 0; set rmargin screen 1; set bmargin screen 0; set tmargin screen 1

set palette defined (0 \"red\", 1 \"green\", 2 \"blue\")

set border 0;
set zeroaxis;
unset xtics; unset ytics;

rgb(r,g,b) = 65536 * int(r) + 256 * int(g) + int(b)
color(x) = (x>0) ? rgb(255,255,255) : rgb(0,0,0)

plot '$3' u 1:2:(\$3) w image, \
'$1' u 1:2:(0.0002) w circles lc rgb \"black\", \
'$2' u 1:2 w filledcurve lc rgb \"#777777\" fs transparent solid 1 border rgb \"black\"\

set label \"t = $time\" at graph 0.05, 0.90 font \"Serif, 26\" 
" | gnuplot

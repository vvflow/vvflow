#!/bin/bash

exec 1>sink.stdout
exec 2>sink.stderr

circle_n12_r1="\
+0.000000 +1.000000 1
-0.500000 +0.866025 1
-0.866025 +0.500000 1
-1.000000 +0.000000 1
-0.866025 -0.500000 1
-0.500000 -0.866025 1
+0.000000 -1.000000 1
+0.500000 -0.866025 1
+0.866025 -0.500000 1
+1.000000 +0.000000 1
+0.866025 +0.500000 1
+0.500000 +0.866025 1"

vvcompose \
	"load" "source" <(echo 0 0 -3.14) \
	"load" "ink_source" <(echo "$circle_n12_r1") \
	"set"  "time" 0 \
	"set"  "dt" 0.001 \
	"set"  "dt_streak" 0.03 \
	"set"  "dt_save" 1 \
	"set"  "time_to_finish" 1 \
	"set"  "caption" "sink" \
	"save" sink.h5 \
&& echo "vvcompose ok" || exit 1

vvflow sink.h5 \
&& echo "vvflow ok" || exit 1

vvplot results_sink/001000.h5 -x -1.5 1.5 --size 400x400 -i ./sink.png \
&& echo "vvplot ok" || exit 1


#!/bin/bash

TMP=$(mktemp -d)
cd $TMP

function raise_error() {
	echo $1
	rm -rf $TMP
	exit 1
}

function pipe_body() {
seq 0 32 | awk 'BEGIN{pi=atan2(0, -1)} {printf "%+0.5f %+0.5f\n", 5+0.5*sin($1*2*pi/64), -0.5*cos($1*2*pi/64)}'
seq +4.95 -0.05 -4.95 | awk '{printf "%+0.5f %+0.5f\n", $1, +0.5}'
seq 0 32 | awk 'BEGIN{pi=atan2(0, -1)} {printf "%+0.5f %+0.5f\n", -5-0.5*sin($1*2*pi/64), 0.5*cos($1*2*pi/64)}'
seq -4.95 +0.05 +4.95 | awk '{printf "%+0.5f %+0.5f\n", $1, -0.5}'
}

vvcompose \
	"load" "source" <(echo -5 0 10; echo 5 0 -10) \
	"load" "body" <(pipe_body) \
	"load" "ink_source" <(seq 0.5 11.5 | awk '{print -5+0.2*cos($1*6.28/12), 0.2*sin($1*6.28/12), 0}') \
	"set"  "time" 1 \
	"set"  "re" 100 \
	"set"  "dt" 0.002 \
	"set"  "dt_save" 1 \
	"set"  "dt_streak" 0.02 \
	"set"  "time_to_finish" 2 \
	"set"  "caption" "slip" \
	"set"  "body00.general_slip" 1 \
	"save" slip.h5 \
|| raise_error "vvcompose failed"

vvflow slip.h5 \
|| raise_error "vvflow failed"

LAST=$(ls results_slip/*.h5 | tail -n1)

vvplot $LAST -x -6 6 -y -1 1 --size 720x $1/pipe_slip.png \
       -i -bvV --blankbody 1 \
|| raise_error "vvplot failed"

cp $LAST $1/pipe_slip.h5

rm -rf $TMP

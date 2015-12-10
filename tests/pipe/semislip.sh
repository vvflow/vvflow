#!/bin/bash

TMP=$(mktemp -d)
cd $TMP

function raise_error() {
	echo $1
	rm -rf $TMP
	exit 1
}

function pipe_body() {
seq 0 16 | awk 'BEGIN{pi=atan2(0, -1)} {printf "%+0.5f %+0.5f\n", 5+0.25*sin($1*2*pi/32), 0.25*cos($1*2*pi/32)}'
seq +4.95 -0.05 -4.95 | awk '{printf "%+0.5f %+0.5f\n", $1, -0.25}'
seq 0 16 | awk 'BEGIN{pi=atan2(0, -1)} {printf "%+0.5f %+0.5f\n", -5-0.25*sin($1*2*pi/32), -0.25*cos($1*2*pi/32)}'
seq -4.95 +0.05 +4.95 | awk '{printf "%+0.5f %+0.5f\n", $1, +0.25}'
}

function pipe_body_bc() {
pipe_body | awk ' {print $0, ($2>0)?1:0}'
}

vvcompose \
	"load" "body" <(pipe_body_bc) \
	"set"  "body00.move.d" -30 \
	"load" "ink_source" <(echo -5 2.5 0) \
	"set"  "time" 0 \
	"set"  "re" 100 \
	"set"  "dt" 0.02 \
	"set"  "dt_save" 1 \
	"set"  "dt_streak" 0.04 \
	"set"  "inf_speed.x" 1 \
	"set"  "time_to_finish" 10 \
	"set"  "caption" "semislip" \
	"save" semislip.h5 \
|| raise_error "vvcompose failed"

vvflow semislip.h5 \
|| raise_error "vvflow failed"

LAST=$(ls results_semislip/*.h5 | tail -n1)

vvplot $LAST -x -6 10 -y -4 4 --size 720x $1/pipe_semislip.png \
       -i -bvV \
|| raise_error "vvplot failed"

cp $LAST $1/pipe_semislip.h5

rm -rf $TMP

#!/bin/bash

TMP=$(mktemp -d)
cd $TMP

function raise_error() {
	echo $1
	rm -rf $TMP
	exit 1
}

vvcompose \
	"load" "source" <(echo 0 0 -3.14) \
	"load" "ink_source" <(echo 0 1 1) \
	"set"  "time" 0 \
	"set"  "re" inf \
	"set"  "dt" 0.001 \
	"set"  "dt_streak" 0.03 \
	"set"  "dt_save" 1 \
	"set"  "time_to_finish" 1 \
	"set"  "caption" "sink" \
	"save" sink.h5 \
|| raise_error "vvcompose failed"

vvflow sink.h5 \
|| raise_error "vvflow failed"

LAST=$(ls results_sink/*.h5 | tail -n1)

vvplot $LAST -x -1.5 1.5 --size 400x400 -i $1/nobody_sink.png \
|| raise_error "vvplot failed"

cp $LAST $1/nobody_sink.h5

rm -rf $TMP

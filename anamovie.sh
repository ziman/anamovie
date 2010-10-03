#!/bin/bash

infile="$1"
tmpdir="$2"

inter_c() {
	ffmpeg -i - -b 800k "$1"
}

inter_d() {
	ffmpeg -i "$1" -f yuv4mpegpipe -
}


mkdir -p "$tmpdir"

dims=$(ffmpeg -i "$1" -vstats 2>&1 | sed -rn 's/.*Stream.* ([0-9]+)x([0-9]+).*/\1:\2/p')
w=${dims%:*}
h=${dims#*:}
w=$((w/2))
x=0

for chan in left right; do
	ffmpeg -i "$infile" -f yuv4mpegpipe -vf crop=$x:0:$w:$h - \
		| ./yuvfilter $chan \
		| inter_c "$tmpdir/$chan.mpg"
	x=$((x+w))
done

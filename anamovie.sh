#!/bin/bash

infile="$1"
outfile="$2"

inter_c() {
	ffmpeg -i - -vcodec mpeg4 -b 800k "$1"
}

inter_d() {
	ffmpeg -i "$1" -f yuv4mpegpipe -
}


dims=$(ffmpeg -i "$1" -vstats 2>&1 | sed -rn 's/.*Stream.* ([0-9]+)x([0-9]+).*/\1:\2/p')
w=${dims%:*}
h=${dims#*:}
w=$((w/2))
x=0

for chan in left right; do
	mkfifo $chan.fifo
	ffmpeg -i "$infile" -f yuv4mpegpipe -vf crop=$x:0:$w:$h - \
		2>/dev/null \
		> $chan.fifo &
	x=$((x+w))
done

rm "$outfile"
./yuvfilter left.fifo right.fifo \
	| inter_c "$outfile"

rm {left,right}.fifo
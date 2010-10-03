#!/bin/bash

infile="$1"
outfile="$2"

dims=$(ffmpeg -i "$1" -vstats 2>&1 | sed -rn 's/.*Stream.* ([0-9]+)x([0-9]+).*/\1:\2/p')
w=${dims%:*}
h=${dims#*:}
w=$((w/2))
x=0

ffmpeg -i "$infile" -acodec copy audio.wav

for chan in left right; do
	mkfifo $chan.fifo
	ffmpeg -i "$infile" -f yuv4mpegpipe -vf crop=$x:0:$w:$h - \
		2>/dev/null \
		> $chan.fifo &
	x=$((x+w))
done

rm "$outfile"
./yuvfilter left.fifo right.fifo \
	| ffmpeg -i - -i audio.wav -acodec copy -vcodec mpeg4 -b 1600k "$outfile"

rm {left,right}.fifo audio.wav
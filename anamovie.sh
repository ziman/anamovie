#!/bin/bash

infile="$1"
tmpdir="$2"

mkdir -p "$tmpdir"

dims=$(ffmpeg -i "$1" -vstats 2>&1 | sed -rn 's/.*Stream.* ([0-9]+)x([0-9]+).*/\1:\2/p')
w=${dims%:*}
h=${dims#*:}
w=$((w/2))

ffmpeg -i "$infile" -f yuv4mpegpipe -vframes 1 -vf crop=0:0:$w:$h -

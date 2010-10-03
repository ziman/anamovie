yuvfilter: yuvfilter.c
	cc -O2 yuvfilter.c -o yuvfilter -W -Wall -std=c99

clean:
	-rm -f yuvfilter *.o *~

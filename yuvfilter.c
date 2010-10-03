#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char uchar;

void die(const char * const msg)
{
	fprintf(stderr, "FATAL: %s\n", msg);
	exit(1);
}

inline void yuv2rgb(int rgb[3], uchar Y, uchar u, uchar v)
{
	int Cr = u - 128L;
	int Cb = v - 128L;
	rgb[0] = Y + Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5);
	rgb[1] = Y - ((Cb >> 2) + (Cb >> 4) + (Cb >> 5)) - ((Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5));
	rgb[2] = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);
}

inline void rgb2yuv(int rgb[3], uchar * Y, uchar * u, uchar * v)
{
}

void processRGB(int rgb[3])
{
	rgb[0] = 0;
}

#define y(p,q) Y[(2*y+p)*width + 2*x+q]
void mogrifyYUVFrame(unsigned char * frame, int width, int height)
{
	uchar * Y = frame;
	uchar * U = Y + width * height;
	uchar * V = U + (width * height) / 4;
	
	for (int y = 0; y < height/2; ++y)
		for (int x = 0; x < width/2; ++x)
		{
			int rgb[4][3];
			int uvOfs = width*y + x;
			uchar u = U[uvOfs];
			uchar v = V[uvOfs];
			yuv2rgb(rgb[0], y(0,0), u, v);
			yuv2rgb(rgb[1], y(0,1), u, v);
			yuv2rgb(rgb[2], y(1,0), u, v);
			yuv2rgb(rgb[3], y(1,1), u, v);

			for (int i = 0; i < 4; ++i)
				processRGB(rgb[i]);

			y(0,0) = y(0,0) / 2;
			y(1,0) = y(1,0) / 2;
		}
}

int main(int argc, char * argv[])
{
	// read YUV format
	char format[2][1024];
	for (int i = 0; i < 2; ++i)
	{
		fgets(format[i], 1024, stdin);
		fprintf(stdout, "%s", format[i]);
	}

	char * w = strchr(format[0], 'W');
	if (!w) die("Could not determine image width.");
	int width = atoi(w+1);

	char * h = strchr(format[0], 'H');
	if (!h) die("Could not determine image height.");
	int height = atoi(h+1);

	int pixels = width * height;
	int frameSize = pixels + pixels/2;
	uchar * frame = (uchar *) malloc(frameSize);
	while (!feof(stdin))
	{
		fread(frame, frameSize, 1, stdin);
		if (feof(stdin)) break;
		
		mogrifyYUVFrame(frame, width, height);
		fwrite(frame, frameSize, 1, stdout);
	}
	free(frame);
	
	return 0;
}

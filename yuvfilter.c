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

inline void yuv2rgb(int rgb[3], int y, int u, int v)
{
	rgb[0] = (9535 * (y - 16) + 13074 * (v - 128)) >> 13;
	rgb[1] = (9535 * (y - 16) -  6660 * (v - 128) - 3203 * (u - 128)) >> 13;
	rgb[2] = (9535 * (y - 16) + 16531 * (u - 128)) >> 13;
}

inline int abs(int x)
{
	return (x < 0) ? -x : x;
}

inline void rgb2yuv(int rgb[3], int yuv[3])
{
	int r = rgb[0]; int g = rgb[1]; int b = rgb[2];
	yuv[0] = abs(r * 2104 + g * 4130 + b * 802 + 4096 + 131072) >> 13;
	yuv[1] = abs(r * (-1214) + g * (-2384) + b * 3598 + 4096 + 1048576) >> 13;
	yuv[2] = abs(r * 3598 + g * -3013 + b * (-585) + 4096 + 1048576) >> 13;
}

inline uchar clamp(int v)
{
	return (v < 0) ? 0 : ((v > 255) ? 255 : (uchar) v);
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
	int ustride = width/2;
	
	for (int y = 0; y < height/2; ++y)
		for (int x = 0; x < width/2; ++x)
		{
			int rgb[4][3];
			int yuv[4][3];
			int uvOfs = ustride*y + x;
			int u = U[uvOfs];
			int v = V[uvOfs];
			yuv2rgb(rgb[0], y(0,0), u, v);
			yuv2rgb(rgb[1], y(0,1), u, v);
			yuv2rgb(rgb[2], y(1,0), u, v);
			yuv2rgb(rgb[3], y(1,1), u, v);

			for (int i = 0; i < 4; ++i)
			{
				processRGB(rgb[i]);
				rgb2yuv(rgb[i], yuv[i]);
			}

		
			y(0,0) = clamp(yuv[0][0]);
			y(0,1) = clamp(yuv[1][0]);
			y(1,0) = clamp(yuv[2][0]);
			y(1,1) = clamp(yuv[3][0]);

			U[uvOfs] = clamp((yuv[0][1] + yuv[1][1] + yuv[2][1] + yuv[3][1]) / 4);
			V[uvOfs] = clamp((yuv[0][2] + yuv[1][2] + yuv[2][2] + yuv[3][2]) / 4);
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

		char separator[1024];
		fgets(separator, sizeof(separator), stdin);
		fprintf(stdout, "%s", separator);
	}
	free(frame);
	
	return 0;
}

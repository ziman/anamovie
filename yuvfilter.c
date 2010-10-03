#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char uchar;

static void die(const char * const msg)
{
	fprintf(stderr, "FATAL: %s\n", msg);
	exit(1);
}

static inline void yuv2rgb(int rgb[3], int y, int u, int v)
{
	rgb[0] = (9535 * (y - 16) + 13074 * (v - 128)) >> 13;
	rgb[1] = (9535 * (y - 16) -  6660 * (v - 128) - 3203 * (u - 128)) >> 13;
	rgb[2] = (9535 * (y - 16) + 16531 * (u - 128)) >> 13;
}

static inline void rgb2yuv(int rgb[3], int yuv[3])
{
	int r = rgb[0]; int g = rgb[1]; int b = rgb[2];
	yuv[0] = abs(r * 2104 + g * 4130 + b * 802 + 4096 + 131072) >> 13;
	yuv[1] = abs(r * (-1214) + g * (-2384) + b * 3598 + 4096 + 1048576) >> 13;
	yuv[2] = abs(r * 3598 + g * -3013 + b * (-585) + 4096 + 1048576) >> 13;
}

static inline uchar clamp(int v)
{
	return (v < 0) ? 0 : ((v > 255) ? 255 : (uchar) v);
}

static inline void processRGB(int left[3], int right[3], int res[3])
{
	res[0] = (right[1] + right[2]) / 2;
	res[1] = left[1];
	res[2] = left[2];
}

#define at_(p,q) [(2*line + (p))*width + 2*col + (q)]
#define at(i) at_(i >> 1, i & 1)
void mogrifyYUVFrames(uchar * left, uchar * right, uchar * res, int width, int height)
{
	int uOfs = width * height;
	int vOfs = uOfs / 4;
	
	for (int i = 0; i < width*height; ++i)
		res[i] = right[i];
	
	uchar * LU = left + uOfs;
	uchar * LV = LU + vOfs;
	uchar * RU = right + uOfs;
	uchar * RV = RU + vOfs;
	uchar * OU = res + uOfs;
	uchar * OV = OU + vOfs;

	int height2 = height/2;
	int width2 = width/2;
	
	for (int line = 0; line < height2; ++line)
		for (int col = 0; col < width2; ++col)
		{
			int lrgb[4][3];
			int rrgb[4][3];
			int rgb[4][3];
			int yuv[4][3];
			for (int i = 0; i < 4; ++i)
			{
				yuv2rgb(lrgb[i], left at(i), *LU, *LV);
				yuv2rgb(rrgb[i], right at(i), *RU, *RV);
				processRGB(lrgb[i], rrgb[i], rgb[i]);
				rgb2yuv(rgb[i], yuv[i]);
				res at(i) = clamp(yuv[i][0]);
			}
		
			*OU++ = clamp((yuv[0][1] + yuv[1][1] + yuv[2][1] + yuv[3][1]) / 4);
			*OV++ = clamp((yuv[0][2] + yuv[1][2] + yuv[2][2] + yuv[3][2]) / 4);
			++LU; ++RU;
			++LV; ++RV;
			
		}
}

void usage()
{
	die(
		"incorrect usage.\n"
		"usage: ./yuvfilter left|right"
	);
}

FILE * sopen(const char * fn, char * format, int size)
{
	FILE * f = fopen(fn, "rb");
	if (!f) return f;
	
	// read YUV format
	fgets(format, size, f);
	fprintf(stdout, "%s", format);

	return f;
}

int getFrame(FILE * f, uchar * frame, int frameSize)
{
	char separator[1024];
	fgets(separator, sizeof(separator), f);
	if (feof(f)) return 1;

	fread(frame, frameSize, 1, f);
	return 0;
}

int main(int argc, char * argv[])
{
	if (argc != 3) usage();

	char format[1024];
	
	FILE * left  = sopen(argv[1], format, sizeof(format));
	if (!left) die("Could not open left file.");
	
	FILE * right = sopen(argv[2], format, sizeof(format));
	if (!right) die("Could not open right file.");
	
	char * w = strstr(format, " W");
	if (!w) die("Could not determine image width.");
	int width = atoi(w+2);

	char * h = strstr(format, " H");
	if (!h) die("Could not determine image height.");
	int height = atoi(h+2);

	char * c = strstr(format, " C");
	if (!c) die("Could not determine pixel format.");
	if (strncmp(c+2, "420", 3)) die("Only yuv420p pixel format supported!");
	
	int pixels = width * height;
	int frameSize = pixels + pixels/2;
	uchar * fLeft  = (uchar *) malloc(frameSize);
	uchar * fRight = (uchar *) malloc(frameSize);
	uchar * fResult= (uchar *) malloc(frameSize);
	
	while (1)
	{
		if (getFrame(left, fLeft, frameSize)) break;
		if (getFrame(right, fRight, frameSize)) break;
		
		mogrifyYUVFrames(fLeft, fRight, fResult, width, height);

		printf("FRAME\n");
		fwrite(fResult, frameSize, 1, stdout);
	}
	
	free(fLeft);
	free(fRight);
	free(fResult);
	
	return 0;
}

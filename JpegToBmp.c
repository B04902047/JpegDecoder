#include <string.h>
#include <stdlib.h>

#include "JpegDecoder.c"
#include "BmpWriter.c"

void getfilePath(char *filePath, char *extension) {
	for (int i=strlen(filePath)-1; i>=0; i--)
		if (filePath[i] == '.') {
			if (strlen(filePath)-i-1 >= strlen(extension) ) {
				strcpy(filePath+i+1, extension);
				filePath[i+1+strlen(extension)] = '\0';
			} else filePath[i] = '\0';
			return;
		}
	filePath[strlen(filePath)-strlen(extension)-1] = '.';
	strcpy(filePath+strlen(filePath)-strlen(extension), extension);
}

unsigned char checkBound(double c) {//cut c into [0,255]
	if (c > 255)
		c = 255;
	else if (c < 0)
		c = 0;
	return c;
}

void YCbCrgetRGBPixel(double *YCbCr, RGBPixel *pixels) {
	double Y = YCbCr[0], Cb = YCbCr[1], Cr = YCbCr[2];
	pixels->BGR[2] = checkBound( Y + 1.402*Cr + 128 );
	pixels->BGR[1] = checkBound( Y - 0.34414*Cb - 0.71414*Cr + 128);
	pixels->BGR[0] = checkBound( Y + 1.772*Cb + 128);
	return;
}

void YCbCrgetRGBPixel2(double *YCbCr, RGBPixel *pixels) {
	double Y = YCbCr[0], Cb = YCbCr[1], Cr = YCbCr[2];
	pixels->BGR[2] = checkBound( 1.164*(Y+112)+1.596*Cr );
	pixels->BGR[1] = checkBound( 1.164*(Y+112)-0.392*Cb-0.813*Cr );
	pixels->BGR[0] = checkBound( 1.164*(Y+112)+2.017*Cb );
	return;
}

int MCUListToRGBMatrix(JpegMCUList *mcuList, RGBMatrix *rgbMatrix) {
	
	rgbMatrix->X = mcuList->X;
	rgbMatrix->Y = mcuList->Y;
	rgbMatrix->pixels
		= (RGBPixel *)malloc(rgbMatrix->X * rgbMatrix->Y
										* sizeof(RGBPixel) );
	void (*getRGBpixel)(double *, RGBPixel *);
	switch (mcuList->componentNumber) {
		case 3:
			getRGBpixel = &YCbCrgetRGBPixel;
			break;
		case 1: case 4: default:
			getRGBpixel = NULL;
			break;
	}
	//TODO:
	//Opt.1: fill RGBmatrix by x and y until X, Y
	unsigned short i, j, k, l;
	double components[ mcuList->componentNumber ];
	unsigned char mcuPositionX, mcuPositionY; //for accel. calculation
	for (int y=rgbMatrix->Y-1; y>=0; y--) //y first
		for (int x=0; x<rgbMatrix->X; x++) {
			i = ( y / mcuList->mcuPixelNumberY )
					* mcuList->mcuNumberX
				+ ( x / mcuList->mcuPixelNumberX );
			mcuPositionY = y % mcuList->mcuPixelNumberY;
			mcuPositionX = x % mcuList->mcuPixelNumberX;
			for (j=0; j< mcuList->componentNumber ; j++) {
				k = ( mcuPositionY
							/ mcuList->componentPixelNumbersY[j] )
						* mcuList->componentSizesX[j]
					+ ( mcuPositionX
							/ mcuList->componentPixelNumbersX[j] );
				l = ( mcuPositionY
								% mcuList->componentPixelNumbersY[j]
							 / (mcuList->Vmax / mcuList->componentSizesY[j]) )
						* JPEG_UNIT_SIZE
					+ ( mcuPositionX
								% mcuList->componentPixelNumbersX[j]
							 / (mcuList->Hmax / mcuList->componentSizesX[j]) );
				components[j]
					= mcuList->MCUs[i].components[j].blocks[k].pixels[l];
			}
			(*getRGBpixel)(components
						, &rgbMatrix->pixels[y*rgbMatrix->X+x]);
		}
	//Opt.2: Use MCUList by i, j, k, l
	//Notice: not over X, Y
	return 1;
}

int jpegToBmp(char *filePath) {

	FILE *fp = fopen(filePath, "rb");
	JpegMCUList mcuList;
	freadJpeg(fp, &mcuList);
	fclose(fp);

	RGBMatrix rgbMatrix;
	MCUListToRGBMatrix(&mcuList, &rgbMatrix);
	freeJpegMCUList(&mcuList);

	getfilePath(filePath, "bmp");
	fp = fopen(filePath, "wb");
	fwriteBmp(fp, &rgbMatrix);
	fclose(fp);

	return 1;
}

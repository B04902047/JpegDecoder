#include <stdio.h>
#include "RGBMatrix.c"

typedef struct {
   unsigned char type[2];                 /* Magic identifier            */
   unsigned int size;                       /* File size in bytes          */
   unsigned short reserved1, reserved2;
   unsigned int offset;                     /* Offset to image data, bytes */
} BmpFileHeader;

typedef struct {
   unsigned int size;               /* Header size in bytes      */
   int width, height;               /* Width and height of image */
   unsigned short int planes;       /* Number of colour planes   */
   unsigned short int bitsPerPixel;	/* Bits per pixel            */
   unsigned int compression;        /* Compression type          */
   unsigned int imageSize;          /* Image size in bytes       */
   int xResolution, yResolution;    /* Pixels per meter          */
   unsigned int colorNumber;		/* Number of colours         */
   unsigned int importantColors;   	/* Important colours         */
} BmpInfoHeader;

int fwriteBmpFileHeader(FILE *fp, BmpFileHeader *fileHeader) {
	char buf[14] = {0};
	buf[0] = fileHeader->type[0];
	buf[1] = fileHeader->type[1];
	buf[2] = (unsigned char)(fileHeader->size	   );
	buf[3] = (unsigned char)(fileHeader->size >>  8);
	buf[4] = (unsigned char)(fileHeader->size >> 16);
	buf[5] = (unsigned char)(fileHeader->size >> 24);
	buf[6] = (unsigned char)(fileHeader->reserved1		);
	buf[7] = (unsigned char)(fileHeader->reserved1 >>  8);
	buf[8] = (unsigned char)(fileHeader->reserved2		);
	buf[9] = (unsigned char)(fileHeader->reserved2>>   8);
	buf[10] = (unsigned char)(fileHeader->offset		);
	buf[11] = (unsigned char)(fileHeader->offset >>    8);
	buf[12] = (unsigned char)(fileHeader->offset >>   16);
	buf[13] = (unsigned char)(fileHeader->offset >>   24);
	return fwrite(buf, 1, 14, fp);
/*
	return fwrite(fileHeader->type, sizeof(unsigned char), 2, fp)
	+ fwrite(&fileHeader->size, sizeof(unsigned int), 1, fp)
	+ fwrite(&fileHeader->reserved1, sizeof(unsigned short), 1, fp)
	+ fwrite(&fileHeader->reserved2, sizeof(unsigned short), 1, fp)
	+ fwrite(&fileHeader->offset, sizeof(unsigned int), 1, fp);
*/
}
int fwriteBmpInfoHeader(FILE *fp, BmpInfoHeader *infoHeader) {
	char buf[40] = {0};
	buf[0] = (unsigned char)(infoHeader->size		);
	buf[1] = (unsigned char)(infoHeader->size >>  8);
	buf[2] = (unsigned char)(infoHeader->size >> 16);
	buf[3] = (unsigned char)(infoHeader->size >> 24);
	buf[4] = (unsigned char)(infoHeader->width		);
	buf[5] = (unsigned char)(infoHeader->width >>  8);
	buf[6] = (unsigned char)(infoHeader->width >> 16);
	buf[7] = (unsigned char)(infoHeader->width >> 24);
	buf[8] = (unsigned char)(infoHeader->height		  );
	buf[9] = (unsigned char)(infoHeader->height >>   8);
	buf[10] = (unsigned char)(infoHeader->height >> 16);
	buf[11] = (unsigned char)(infoHeader->height >> 24);
	buf[12] = (unsigned char)(infoHeader->planes	  );
	buf[13] = (unsigned char)(infoHeader->planes >>  8);
	buf[14] = (unsigned char)(infoHeader->bitsPerPixel	   );
	buf[15] = (unsigned char)(infoHeader->bitsPerPixel >> 8);
	return fwrite(buf, 1, 40, fp);
}

int fwriteBmpBody(FILE *fp, RGBMatrix *rgbMatrix) {
	unsigned int padSize = (4 - rgbMatrix->X * 3 % 4) % 4;	//%4 necessary?
	unsigned char bmpPad[3] = {0,0,0};
	for(int y=rgbMatrix->Y-1; y>=0; y--) {
		for (int x=0; x<rgbMatrix->X; x++)
			fwrite(rgbMatrix->pixels[y*rgbMatrix->X+x].BGR, 1, 3, fp);
		fwrite(bmpPad, 1, padSize, fp);
	}
	return 1;
}
//waiting for generalization to non-RGB bmps
int fwriteBmp(FILE *fp, RGBMatrix *rgbMatrix) {

	unsigned int padSize = (4 - rgbMatrix->X * 3 % 4) % 4;

	BmpFileHeader fileHeader;
	memset(&fileHeader, 0, sizeof(BmpFileHeader));
	fileHeader.type[0] = 'B';
	fileHeader.type[1] = 'M';
	fileHeader.size = 14 + 40
						+ 3 * (rgbMatrix->X) * (rgbMatrix->Y)
						+ padSize * (rgbMatrix->Y);
	fileHeader.offset = 14 + 40;

	BmpInfoHeader infoHeader;
	memset(&infoHeader, 0, sizeof(BmpInfoHeader));
	infoHeader.size = 40;
	infoHeader.width = rgbMatrix->X;
	infoHeader.height = rgbMatrix->Y;
	infoHeader.planes = 1;
	infoHeader.bitsPerPixel = 24;

	fwriteBmpFileHeader(fp, &fileHeader);
	fwriteBmpInfoHeader(fp, &infoHeader);
	fwriteBmpBody(fp, rgbMatrix);

	free(rgbMatrix->pixels);
	fclose(fp);
	return 1;
}

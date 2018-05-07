#ifndef _JPEG_MCU_
#define _JPEG_MCU_

#include <stdlib.h>
#include "JpegHeader.c"

#define JPEG_UNIT_SIZE 8//64 bit per block

typedef struct {
	short coefficients[64];
	double pixels[64];
} Block;

typedef struct {
	Block *blocks;
} Blocks;

typedef struct {
	Blocks *components;
} JpegMCU;

typedef struct {
	JpegMCU *MCUs;
	unsigned char *componentTreeIndices;
	unsigned char *componentSizesX;	//Xblocks per component
	unsigned char *componentSizesY;
	unsigned char *componentSizes;
	unsigned char *componentPixelNumbersX;	//Xpixels per component block
	unsigned char *componentPixelNumbersY;
	unsigned char componentNumber;
	unsigned char Hmax;
	unsigned char Vmax;
	unsigned char mcuPixelNumberX;	//Xpixels per MCU
	unsigned char mcuPixelNumberY;
	//unsigned char mcuBlockNumber;	//not used
	unsigned short mcuNumberX;
	unsigned short mcuNumberY;
	unsigned short mcuNumber;
	unsigned short X;
	unsigned short Y;
} JpegMCUList;

/*
unsigned char getMCUBlockNumber(JpegSOF *SOF) {
	unsigned char count = 0;
	for (int i=0; i<SOF->Nf; i++)
		count += (SOF->components[i].H) * (SOF->components[i].V);
	return count;
}
*/

void getComponentPixelNumbers(JpegMCUList *ptr) {
	ptr->componentPixelNumbersX = malloc(ptr->componentNumber
										* sizeof(unsigned char));
	ptr->componentPixelNumbersY = malloc(ptr->componentNumber
										* sizeof(unsigned char));
	for (int i=0; i<ptr->componentNumber; i++) {
		ptr->componentPixelNumbersX[i]
			= ptr->mcuPixelNumberX / ptr->componentSizesX[i];
		ptr->componentPixelNumbersY[i]
			= ptr->mcuPixelNumberY / ptr->componentSizesY[i];
	}
	return;
}

void getComponentSizes(JpegSOF *SOF, JpegMCUList *ptr) {
	ptr->componentSizesX
		= (unsigned char *)malloc( SOF->Nf * sizeof(unsigned char) );
	ptr->componentSizesY
		= (unsigned char *)malloc( SOF->Nf * sizeof(unsigned char) );
	ptr->componentSizes
		= (unsigned char *)malloc( SOF->Nf * sizeof(unsigned char) );
	for (int i=0; i<SOF->Nf; i++) {
		ptr->componentSizesX[i] = (SOF->components[i].H);
		ptr->componentSizesY[i] = (SOF->components[i].V);
		ptr->componentSizes[i] = ptr->componentSizesX[i]
									* ptr->componentSizesY[i];
	}
	return;
}

void getComponentTreeIndices(JpegMCUList *ptr) {
	 
	switch (ptr->componentNumber) {
		case 3:
			ptr->componentTreeIndices
				= (unsigned char *)malloc(
							3 * sizeof(unsigned char) );
			ptr->componentTreeIndices[0] = 0;
			ptr->componentTreeIndices[1]
				= ptr->componentTreeIndices[2]
				= 1;
			return;
		case 1: case 4: default:
			ptr->componentTreeIndices = NULL;
			return;
	}
}

unsigned short ceilingDivide(unsigned short a, unsigned short b) {
	return (a+b-1)/ b;
}

unsigned char getVmax(JpegSOF *SOF) {
	unsigned char max = 0;
	for (int i=0; i<SOF->Nf; i++)
		if (SOF->components[i].V > max)
			max = SOF->components[i].V;
	return max;
}

unsigned char getHmax(JpegSOF *SOF) {
	unsigned char max = 0;
	for (int i=0; i<SOF->Nf; i++)
		if (SOF->components[i].H > max)
			max = SOF->components[i].H;
	return max;
}

int initJpegMCUList(JpegMCUList *ptr, JpegSOF *SOF) {

	ptr->X = SOF->X;
	ptr->Y = SOF->Y;
	ptr->Hmax = getHmax(SOF);
	ptr->Vmax = getVmax(SOF);

	ptr->mcuPixelNumberX = ptr->Hmax * JPEG_UNIT_SIZE;
	ptr->mcuPixelNumberY = ptr->Vmax * JPEG_UNIT_SIZE;

	ptr->mcuNumberX = ceilingDivide( ptr->X, ptr->mcuPixelNumberX );
	ptr->mcuNumberY = ceilingDivide( ptr->Y, ptr->mcuPixelNumberY );
	ptr->mcuNumber = ptr->mcuNumberX * ptr->mcuNumberY;

	ptr->componentNumber = SOF->Nf;
	getComponentTreeIndices(ptr);
	getComponentSizes(SOF, ptr);
	getComponentPixelNumbers(ptr);
	//ptr->mcuBlockNumber = getMCUBlockNumber(SOF);

	ptr->MCUs = (JpegMCU *)malloc( sizeof(JpegMCU) * ptr->mcuNumber );
	for (int i=0; i<ptr->mcuNumber; i++) {
		ptr->MCUs[i].components
			= (Blocks *)malloc( sizeof(Blocks)
									* ptr->componentNumber );
		for (int j=0; j<ptr->componentNumber; j++)
			ptr->MCUs[i].components[j].blocks
				= (Block *)malloc( sizeof(Block)
									* ptr->componentSizes[j] );
	}
	return 1;
}

int freeJpegMCUList(JpegMCUList *ptr) {
	//TODO: recursively free all the pointers;
	//free(ptr);
	return 1;
}

#endif
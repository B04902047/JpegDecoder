#include <stdio.h>

#include "JpegHeader.c"
#include "JpegMCU.c"

#include "JpegHuffman.c"
#include "FastIDCT.c"


int deDCdiff(JpegMCUList *mcuList) {
	short previousDC;
	for (int j=0; j<mcuList->componentNumber; j++) {//j first
		previousDC = 0;
		for (int i=0; i<mcuList->mcuNumber; i++)
			for (int k=0; k<mcuList->componentSizes[j]; k++) {
				previousDC
				= mcuList->MCUs[i].components[j].blocks[k].coefficients[0]
				+= previousDC;
			}
	}
logMsg("deDCdiffed");
	return 1;
}

int deZigzag(JpegMCUList *mcuList) {
	short newCoefficients[64];
	unsigned char zigzag[64]
						= {0,  1,  5,  6, 14, 15, 27, 28
						,  2,  4,  7, 13, 16, 26, 29, 42
						,  3,  8, 12, 17, 25, 30, 41, 43
						,  9, 11, 18, 24, 31, 40, 44, 53
						, 10, 19, 23, 32, 39, 45, 52, 54
						, 20, 22, 33, 38, 46, 51, 55, 60
						, 21, 34, 37, 47, 50, 56, 59, 61
						, 35, 36, 48, 49, 57, 58, 62, 63};
	for (int i=0; i<mcuList->mcuNumber; i++)
		for (int j=0; j<mcuList->componentNumber; j++)
			for (int k=0; k<mcuList->componentSizes[j]; k++) {
				for (int l=0; l<64; l++)
					newCoefficients[l]
						= mcuList->MCUs[i].components[j].blocks[k].coefficients[zigzag[l]];
				for (int l=0; l<64; l++)
					mcuList->MCUs[i].components[j].blocks[k].coefficients[l]
						= newCoefficients[l];
			}
logMsg("deZigzagged");
	return 1;
}

int fgetJpegMCUList(FILE *fp, JpegMCUList *mcuList, JpegSOF *SOF, JpegDHT *DHT) {
	return initJpegMCUList(mcuList, SOF) //check if mcuList is healthy?
		&& fdecodeHuffman(fp, mcuList, DHT)	
		&& deDCdiff(mcuList);
}

int deQuantize(JpegMCUList *mcuList, JpegSOF *SOF, JpegDQT *DQT) {

	for (int i=0; i<mcuList->mcuNumber; i++)
		for (int j=0; j<mcuList->componentNumber; j++)
			for (int k=0; k<mcuList->componentSizes[j]; k++) {
				for (int l=0; l<64; l++) {
					mcuList->MCUs[i].components[j].blocks[k].coefficients[l]
						*= DQT[SOF->components[j].Tq].Q[l];
				}
			}
logMsg("deQuantized");
	return 1;
}

int deDCT(JpegMCUList *mcuList) {

	double alphaCos[JPEG_UNIT_SIZE][JPEG_UNIT_SIZE];
	getAlphaCos(JPEG_UNIT_SIZE, alphaCos);

	for (int i=0; i<mcuList->mcuNumber; i++)
		for (int j=0; j<mcuList->componentNumber; j++)
			for (int k=0; k<mcuList->componentSizes[j]; k++) {
				FastIDCT (
					JPEG_UNIT_SIZE
					, alphaCos
					, mcuList->MCUs[i].components[j].blocks[k].coefficients
					, mcuList->MCUs[i].components[j].blocks[k].pixels
				);
			}
logMsg("deDCT'ed");
	return 1;
}

int fdecodeJpegBody(FILE *fp, JpegMCUList *mcuList
									, JpegHeader *header) {
logMsg("Start scanning...");
	return fgetJpegMCUList(fp, mcuList, &header->SOF, header->DHT)
		&& deQuantize(mcuList, &header->SOF, header->DQT)
		&& deZigzag(mcuList)
		&& deDCT(mcuList);
}

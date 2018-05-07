#include <assert.h>

#include "JpegMCU.c"
#include "HuffmanTree.c"
#include "BitOperationTools.c"
#include "JpegHeader.c"

#include "ErrorHandler.c"

//not dealing with RSTn

unsigned char fgetHuffmanSymbol(FILE *fp, HuffmanTreeNode *treeHead
									, BitStorage *bitStorage) {
	HuffmanTreeNode *ptr = treeHead;
	while( fgetBit(fp, bitStorage) ) {
		ptr = ptr->child[bitStorage->bits];
		if (ptr == NULL) {
			errorMsg("fgetHuffmanSymbol: search to NULL");
			return 0;
		} else if (ptr->isLeaf) {
			return ptr->symbol;
		}
	}
	errorMsg("fgetHuffmanSymbol: EOF");
	return 0;
}

short fdecodeDCdiff(FILE *fp, HuffmanTreeNode *treeHead
									, BitStorage *bitStorage) {
	unsigned char size
		= fgetHuffmanSymbol(fp, treeHead, bitStorage);
	fgetBits(fp, bitStorage, size);
	return extendBits(bitStorage->bits, size);
}

void runLengthFillArray(unsigned char run, short length
						, short *AC, unsigned filled) {
	for (int i=0; i<run; i++)
		AC[filled+i] = 0;
	AC[filled+run] = length;
	return;
}

//is it possible that this function fail? for debug?
void fdecodeAC(FILE *fp, HuffmanTreeNode *treeHead
					, BitStorage *bitStorage, short *AC) {
	unsigned char codeWord, run, size;
	short length;
	unsigned char filled = 0;	//number of ACs filled: 0~63
	while(filled < 63) {
		codeWord = fgetHuffmanSymbol(fp, treeHead, bitStorage);
		if (!codeWord) {
			run = 63-filled-1;
			length = size = 0;
		} else {
			splitByte(codeWord, &run, &size);
			fgetBits(fp, bitStorage, size);
			length = extendBits(bitStorage->bits, size);
		}
//printf("RRRR %4d|\t SSSS %4d| length %4d\n", run, size, length);
		runLengthFillArray(run, length, AC, filled);
		filled += run + 1;
	}

	if (filled != 63) {	//waiting for debug/ error handle
		printf("error: %d filled\n", filled);
		assert(0);
	}

	return;
}

int fdecodeHuffman(FILE *fp, JpegMCUList *mcuList, JpegDHT *DHT) {
	unsigned char *treeIndices	//abbrev.
					= mcuList->componentTreeIndices;
	BitStorage *bitStorage = newBitStorage();
	for (int i=0; i<mcuList->mcuNumber; i++)
		for (int j=0; j<mcuList->componentNumber; j++)
			for (int k=0; k<mcuList->componentSizes[j]; k++) {
				mcuList->MCUs[i].components[j].blocks[k].coefficients[0]
					= fdecodeDCdiff(fp
						, DHT[treeIndices[j]*2].treeHead
						, bitStorage);
				fdecodeAC(fp
					, DHT[treeIndices[j]*2+1].treeHead
					, bitStorage
					, mcuList->MCUs[i].components[j].blocks[k].coefficients+1);
			}
logMsg("Huffman Decoded");
	return freadToJpegEOI(fp);
}

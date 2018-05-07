#ifndef _JPEG_HEADER_
#define _JPEG_HEADER_

#include <stdio.h>
#include <stdlib.h>

#include "BitOperationTools.c"
#include "HuffmanTree.c"
#include "ErrorHandler.c"

typedef struct {
	unsigned char C;
	unsigned char H;	//4-bit
	unsigned char V;	//4-bit
	unsigned char Tq;
} SOFComponent;

typedef struct {
	unsigned char	P;
	unsigned short	Y;
	unsigned short	X;
	unsigned char	Nf;
	SOFComponent *components;
} JpegSOF;

typedef struct {
	unsigned char 	Cs;
	unsigned char	Td;
	unsigned char	Ta;	//4-bit
} SOSComponent;

typedef struct {
	unsigned char	Ns;
	SOSComponent *components;
	unsigned char	Ss;
	unsigned char	Se;
	unsigned char	Ah;	//4-bit
	unsigned char	Al;	//4-bit
} JpegSOS;

typedef struct {
	unsigned char	Pq; //4-bit
	unsigned char	Tq; //4-bit
	unsigned short	Q[64];	//8 or 16 bit according to Pq
} JpegDQT;

typedef struct {
	HuffmanTreeNode	*treeHead;
	unsigned char	L[16];
	unsigned char	*V[16];
} JpegDHT;

typedef struct {
	JpegSOF			SOF;
	JpegSOS			SOS;
	JpegDQT			*DQT;
	JpegDHT			*DHT;
	//SOI;
	//EOI;
	//DRI;
	//APP[n];
	//EOI;
} JpegHeader;

int initJpegHeader(JpegHeader *header) {
	header->SOF.components = NULL;
	header->SOS.components = NULL;
	header->DQT = (JpegDQT *)malloc( 4 * sizeof(JpegDQT) );
	header->DHT = (JpegDHT *)malloc( 4 * sizeof(JpegDHT) );
	return 1;
}

int freeJpegHeader(JpegHeader *header) {
	//TODO: recursively free all the pointers inside
	//free(header);
	//logMsg("JpegHeader freed");
	return 1;
}

int fparseJpegSOF(FILE *fp, JpegSOF *SOF) {

	unsigned short readByteCount, dataLength;
	readByteCount = fread(&dataLength, 1, 2, fp);
	reverseEndian(&dataLength);
	readByteCount += fread(&SOF->P, 1, 1, fp)
				+ fread(&SOF->Y, 1, 2, fp)
				+ fread(&SOF->X, 1, 2, fp)
				+ fread(&SOF->Nf, 1, 1, fp);
	reverseEndian(&SOF->Y);
	reverseEndian(&SOF->X);

	SOF->components
		= (SOFComponent *)malloc( sizeof(SOFComponent) * SOF->Nf );
	for (int i=0; i < SOF->Nf; i++) {
		unsigned char temp = 0;
		readByteCount +=
			fread(&SOF->components[i].C, 1, 1, fp)
			+ fread(&temp, 1, 1, fp)
			+ fread(&SOF->components[i].Tq, 1, 1, fp);
		splitByte(temp
			, &SOF->components[i].H, &SOF->components[i].V);
	}
	return (readByteCount == dataLength);
}

int fparseJpegSOS(FILE *fp, JpegSOS *SOS) {

	unsigned short readByteCount, dataLength;
	readByteCount = fread(&dataLength, 1, 2, fp);
	reverseEndian(&dataLength);
	readByteCount += fread(&SOS->Ns, 1, 1, fp);

	unsigned char temp;
	SOS->components
		= (SOSComponent *)malloc( sizeof(SOSComponent) * SOS->Ns );
	for (int i=0; i<SOS->Ns; i++) {
		temp = 0;
		readByteCount
			+= fread(&SOS->components[i].Cs, 1, 1, fp)
			+ fread(&temp, 1, 1, fp);
		splitByte(temp
			, &SOS->components[i].Td, &SOS->components[i].Ta);
	}
	temp = 0;
	readByteCount
		+= fread(&SOS->Ss, 1, 1, fp)
		+ fread(&SOS->Se, 1, 1, fp)
		+ fread(&temp, 1, 1, fp);
	splitByte(temp, &SOS->Ah, &SOS->Al);
	return (readByteCount == dataLength);
}

int fparseJpegDQT(FILE *fp, JpegDQT *DQT) {

	unsigned short readByteCount, dataLength;
	readByteCount = fread(&dataLength, 1, 2, fp);
	reverseEndian(&dataLength);

	unsigned char temp, precision, index;
	int tableCount = 0;
	while ( readByteCount < dataLength ) {
		readByteCount += fread(&temp, 1, 1, fp);
		splitByte(temp, &precision, &index);

		DQT[index].Pq = precision;
		DQT[index].Tq = index;	//could be ignore?
		for (int j=0; j<64; j++) {
			readByteCount += fread(&DQT[index].Q[j]
									, 1, precision+1, fp);
			if (precision)
				reverseEndian(&DQT[index].Q[j]);
		}
		tableCount++;
	}
	return (tableCount <= 4) && (readByteCount == dataLength);
}

int JpegDHTcreateHuffmanTree(JpegDHT *DHT) {
	DHT->treeHead = newHuffmanTreeNode();

	unsigned char leavesPutCount = 0, numberOfLeavesPut;
	for (int i=0; i<16; i++) {
		HuffmanTreeNode *leaves
			= newHuffmanTreeLeaves(DHT->L[i], DHT->V[i]);
		numberOfLeavesPut = putLeavesUnderHuffmanTreeNode(
								DHT->treeHead
								, leaves
								, DHT->L[i]
								, i+1
							);
		if ( DHT->L[i] != numberOfLeavesPut) {
			errorMsg("JpegDHTcreateHuffmanTree: numberOfLeavesPut incorrect");
			return 0;
		}
	}
	return 1;
}

//return treeIndex (Tc + Th*2) if success else -1
int fparseJpegDHT(FILE *fp, JpegDHT *DHT) {

	unsigned short readByteCount, dataLength;
	readByteCount = fread(&dataLength, 1, 2, fp);
	reverseEndian(&dataLength);

	unsigned char treeType, Tc, Th;
	int tableCount = 0, treeIndex;
	while (readByteCount < dataLength) {
		readByteCount += fread(&treeType, 1, 1, fp);
		splitByte(treeType, &Tc, &Th);
		treeIndex = Tc + Th*2;
		for (int i=0; i<16; i++)
			readByteCount += fread(&DHT[treeIndex].L[i], 1, 1, fp);

		for (int i=0; i<16; i++) {
			DHT[treeIndex].V[i]
				= (unsigned char *) malloc ( DHT[treeIndex].L[i]
												* sizeof(unsigned char) );
			readByteCount += fread(DHT[treeIndex].V[i], 1
										, DHT[treeIndex].L[i], fp);

		}
		if (! JpegDHTcreateHuffmanTree ( &DHT[ treeIndex ] ) ) {
			errorMsg("JpegDHTcreateHuffmanTree");
			return 0;
		};
		tableCount++;
	}
	if (tableCount > 4 || readByteCount != dataLength)
		return 0;
	return 1;
}

int fparseJpegAPP(FILE *fp) {
	unsigned short dataLength;
	reverseEndian(&dataLength);
	unsigned char temp;
	for (int i=0; i<dataLength; i++)
		fread(&temp, 1, 1, fp);
	errorMsg("APP header discarded");
	return 1;
}

unsigned char fgetJpegMarker(FILE *fp) {
	unsigned char marker;
	while ( fread(&marker, 1, 1, fp) )
		if (marker == 0xFF)
			while ( fread(&marker, 1, 1, fp) )
				if (marker != 0xFF)
					return marker;
	return 0xFF;
}

int freadToJpegMarker(FILE *fp, unsigned char marker) {
	unsigned char result;
	while (1) {
		result = fgetJpegMarker(fp);
		if (result == marker)
			return 1;
		if (result == 0xFF)
			return 0;
	}
}

//return 1 if success else 0
int freadToJpegSOI(FILE *fp) {
	if ( freadToJpegMarker(fp, 0xD8) ) {
		logMsg("marker: SOI");
		return 1;
	}
	return 0;
}
//return 1 if success else 0
int freadToJpegEOI(FILE *fp) {
	if ( freadToJpegMarker(fp, 0xD9) ) {
		logMsg("marker: EOI");
		return 1;
	}
	return 0;
}

//return 1 for success else 0
int fparseJpegHeader(FILE *fp, JpegHeader *header) {

	freadToJpegSOI(fp);

	unsigned char marker;
	int treeIndex;
	while(1) {
		marker = fgetJpegMarker(fp);
		//printf("logMsg: marker: %X\n", marker);
		switch( marker ) {
			case 0xD9:	//EOI: end of image
				return 0;
			case 0xDD:	//DRI: define restart interval
			case 0xD0:	//RSTm: m = 0~7
			case 0xC1:	//SOF1
				errorMsg("SOF1");
				break;
			case 0xE0: case 0xE1: case 0xE2: case 0xE3://APPn: n = 0~f
			case 0xE4: case 0xE5: case 0xE6: case 0xE7:
			case 0xE8: case 0xE9: case 0xEA: case 0xEB:
			case 0xEC: case 0xED: case 0xEE: case 0xEF:
				fparseJpegAPP(fp);
				break;
			case 0xDB:	//DQT: define quantization table
				if ( !fparseJpegDQT(fp, header->DQT) ) {
					errorMsg("fparseJpegDQT");
					return 0;
				}
				break;
			case 0xC0:	//SOF: start of frame
				if (!fparseJpegSOF(fp, &header->SOF) ) {
					errorMsg("fparseJpegSOF");
					return 0;
				}
				break;
			case 0xC4:	//DHT: define Huffman table
				switch (header->SOF.Nf) {
					//check color type: Gray Scale/ YCbCr/ CMYK
					//possible modification: put this checking process into fparseJpegDHT()
					case 3:
						if ( !fparseJpegDHT(fp, header->DHT) ) {
							errorMsg("fparseJpegDHT");
							return 0;
						}
						break;
					case 1: case 4: default:
						return 0;
				}
				break;
			case 0xDA:	//SOS: start of scan
				if ( !fparseJpegSOS(fp, &header->SOS) ) {
					errorMsg("fparseJpegSOS");
					return 0;
				}
				else return 1;
				break;
			default:
				break;
		}
	}
}

#endif
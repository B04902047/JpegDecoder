#ifndef _BIT_OPERATION_TOOLS_
#define _BIT_OPERATION_TOOLS_

#include <stdio.h>
#include <stdlib.h>
#include "ErrorHandler.c"

void reverseEndian(unsigned short *word) {
	*word = ( *word>>8 | *word<<8 );
	return;
}

void splitByte(unsigned char temp, unsigned char *high, unsigned char *low) {
	*high = temp >> 4;
	*low = temp & 0x0f;
	return;
}

typedef struct {
	unsigned char size;
	unsigned char byte;
	short bits;	//at most 15
} BitStorage;

BitStorage *newBitStorage() {
	BitStorage *ptr = (BitStorage *) malloc( sizeof(BitStorage) );
	ptr->size = 0;
	ptr->byte = 0;
	ptr->bits = 0;
	return ptr;
}

void printByte(unsigned char byte, unsigned char size) {
	for (int i=0; i<size; i++, byte<<=1)
		printf("%d", (byte & 0x80)>>7);
	printf("\n");
}

int fgetBit(FILE *fp, BitStorage *bitStorage) {
//printf("sizeLeft: %d\n", bitStorage->size);
	if ( bitStorage->size == 0 ) {
		if ( !fread(&bitStorage->byte, 1, 1, fp) ) {
			errorMsg("fgetBit: EOF");
			return 0;
		}
		while (bitStorage->byte == 0xFF) {
			//debugMsg("fgetBit: 0xFF");
			if ( !fread(&bitStorage->byte, 1, 1, fp) ) {
				errorMsg("fgetBit: EOF");
				return 0;
			}
			if (bitStorage->byte == 0xFF) {
				//logMsg("continue: 0xFF");
				continue;
			} else if (bitStorage->byte == 0x00) {
				//logMsg("fgetBit: 0x00");
				bitStorage->byte = 0xFF;
				break;
			} else if (bitStorage->byte == 0xD9) {
				errorMsg("fgetBit: EOI");
				return 0;
			} else {
				errorMsg("fgetBit: 0xFF, 0x??");
				return 0;
			}
		}
		bitStorage->size = 8;
//printf("-- read byte --\n");
	}
//printf("byteLeft: ");
//printByte(bitStorage->byte, bitStorage->size);

/*old
	bitStorage->bits = bitStorage->byte & 0x01;
//printf("get %d\n", bitStorage->bits);
	bitStorage->byte >>= 1;
*/
//new
	bitStorage->bits = (bitStorage->byte >> 7);//& 0x01;
//printf("bits: %d\n", bitStorage->bits);
	bitStorage->byte <<= 1;
//
	bitStorage->size --;
//debugMsg("fgetBit returns");
	return 1;
}

unsigned char min(unsigned char a, unsigned char b) {
	return (a<b)? a:b;
}


//return the number of bits gotten
unsigned char fgetBits(FILE *fp, BitStorage *bitStorage
									, unsigned char size) {
	short length = 0;
	for (int i=size-1; i>=0; i--) {
		if ( !fgetBit(fp, bitStorage) )
			return size-i;
		length += bitStorage->bits << i;
	}
	bitStorage->bits = length;
	return size;
}

short extendBits(short bits, unsigned char size) {
	if ( (size) && (bits < ( 1 << (size-1) )) )
		bits = bits - (1<<size) + 1;
	return bits;
}

#endif
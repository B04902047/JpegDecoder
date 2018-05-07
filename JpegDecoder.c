#include <stdio.h>
#include "JpegHeader.c"
#include "JpegBody.c"
#include "JpegMCU.c"

int freadJpeg(FILE *fp, JpegMCUList *mcuList) {
	JpegHeader header;
	initJpegHeader(&header);
	if ( fparseJpegHeader(fp, &header) 
		&& fdecodeJpegBody(fp, mcuList, &header) )
		return freeJpegHeader(&header);
	return 0;
}

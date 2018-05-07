#ifndef _ERROR_HANDLER_
#define _ERROR_HANDLER_

#include <stdio.h>

void errorMsg(const char *str) {
	//printf("errorMsg: %s\n", str);
	return;
}

void logMsg(const char *str) {
	//printf("logMsg: %s\n", str);
	return;
}

void debugMsg(const char *str) {
	//printf("debugMsg: %s\n", str);
	return;
}

void printMatrix(short *ptr) {
	for (int i=0; i<8; i++)
		for (int j=0; j<8; j++)
			printf("%4d%c", ptr[i*8+j], (j==7)? '\n':' ');
	printf("---------------\n");
	return;
}

void printDoubleMatrix(double *ptr) {
	for (int i=0; i<8; i++)
		for (int j=0; j<8; j++)
			printf("%4.0f%c", ptr[i*8+j], (j==7)? '\n':' ');
	printf("---------------\n");
	return;
}

#endif
#include "JpegToBmp.c"

int main(int argc, char *argv[]) {
	for (int i=1; i<argc; i++) {
		printf("File %d: %s\n", i, argv[i]);
		jpegToBmp(argv[i]);
	}
	return 0;
}

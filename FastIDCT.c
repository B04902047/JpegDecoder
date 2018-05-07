#include <math.h>

void getAlphaCos(unsigned char N, double alphaCos[N][N]) {
	double alpha;
	for (int x=0; x<N; x++)
		for (int i=0; i<N; i++) {
			alpha = (i==0)? 1/sqrt(2):1;
			alphaCos[x][i] = alpha * cos( (2*x+1)*i*M_PI/16 ) / 2;
		}
	return;
}

void FastIDCT(unsigned char N, double alphaCos[N][N]
				, short coefficents[N][N], double pixels[N][N]) {
	double temp[N][N];
	for (int x=0; x<N; x++)
		for (int y=0; y<N; y++) {
			temp[x][y] = 0;
			for (int j=0; j<N; j++)
				temp[x][y] += coefficents[x][j] * alphaCos[y][j];
		}
	for (int y=0; y<N; y++)
		for (int x=0; x<N; x++) {
			pixels[x][y] = 0;
			for (int i=0; i<N; i++)
				pixels[x][y] += temp[i][y] * alphaCos[x][i];
		}
	return;
}

void FastIDCT2(unsigned char N, double alphaCos[N][N]
				, short coefficents[N][N], double pixels[N][N]) {
	double temp[N][N];
	for (int y=0; y<N; y++)
		for (int x=0; x<N; x++) {
			temp[x][y] = 0;
			for (int i=0; i<N; i++)
				temp[x][y] += coefficents[i][y] * alphaCos[x][i];
		}
	for (int x=0; x<N; x++)
		for (int y=0; y<N; y++) {
			pixels[x][y] = 0;
			for (int j=0; j<N; j++)
				pixels[x][y] += temp[x][j] * alphaCos[y][j];
		}
	return;
}

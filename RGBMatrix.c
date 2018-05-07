
typedef struct {
	unsigned char BGR[3];	//{B, G, R};
} RGBPixel;

typedef struct {
	unsigned short X, Y;
	RGBPixel *pixels;
} RGBMatrix;

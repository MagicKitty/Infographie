#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

void line(int x1, int y1, int x2, int y2, TGAImage &image, TGAColor color) {
	x1 = x1 + x2;
	x2 = x1 - x2;
	int tmp = 0;
	if(x1 > x2) {
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if(y1 > y2) {
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	int dx = x2 - x1;
	int dy = y2 - y1;
	int j = 0;
	for(int i = x1; i < x2; i++) {
		j = y1+dy*(i-x1)/dx;
		image.set(i, j, color);
	}
}

int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);
	line(40,20,10,40,image,white);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}


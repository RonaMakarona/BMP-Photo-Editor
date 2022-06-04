#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>

#define  Pr  .299
#define  Pg  .587
#define  Pb  .114

typedef unsigned char char8;

struct BITMAP_header {
	char name[2]; // BM
	uint32_t size;
	int garbage; // ?
	uint32_t image_offset; //offset from where the image starts in the file
};

struct DIB_header {
	uint32_t header_size;
	int32_t width;
	int32_t height;
	uint16_t colorplanes;
	uint16_t bitsperpixel;
	uint32_t compression;
	uint32_t image_size;
	uint32_t temp[4];
};


struct RGB
{
	char8 blue;
	char8 green;
	char8 red;
};

struct Image {
	struct RGB** rgb;
	int height;
	int width;
};

// declare globals
struct BITMAP_header main_header;
struct DIB_header main_dibheader;
struct Image main_pic;

void readint(FILE* f, int* val, int size)
{
	unsigned char buf[4];
	fread(buf, size, 1, f);
	*val = 0;
	for (int i = size - 1; i >= 0; i--)
		*val += (buf[i] << (8 * i));
}

void writeint(FILE* f, int val, int size)
{
	unsigned char buf[4];
	for (int i = size - 1; i >= 0; i--)
		buf[i] = (val >> 8 * i) & 0xff;
	fwrite(buf, size, 1, f);
}

struct Image readImage(FILE* fp, int height, int width) {
	struct Image pic;

	pic.rgb = (struct RGB**)malloc(height * sizeof(void*)); // pointer to a row  of rgb data (pixels)
	pic.height = height;
	pic.width = width;

	for (int i = height - 1; i >= 0; i--)
	{
		pic.rgb[i] = (struct RGB*)malloc(width * sizeof(struct RGB)); // allocating a row of pixels
		fread(pic.rgb[i], width, sizeof(struct RGB), fp);
	}

	return pic;
}

void freeImage(struct Image pic) {
	for (int i = pic.height - 1; i >= 0; i--)
	{
		free(pic.rgb[i]);
	}

	free(pic.rgb);
}

unsigned char grayscale(struct RGB rgb) {
	return ((0.3 * rgb.red) + (0.6 * rgb.green) + (0.1 * rgb.blue)) / 3;
}

struct RGB changeBrightness(struct RGB rgb, float i) {

	if (i >= 0) {
		rgb.red = min(rgb.red + i, 255);
		rgb.green = min(rgb.green + i, 255);
		rgb.blue = min(rgb.blue + i, 255);
	}
	if (i < 0) {
		rgb.red = max(rgb.red + i, 0);
		rgb.green = max(rgb.green + i, 0);
		rgb.blue = max(rgb.blue + i, 0);
	}

	return rgb;
}

void RGBtoGrayscale(struct BITMAP_header header, struct DIB_header dibheader, struct Image pic) {

	FILE* fpw = fopen(tmpPath, "wb");
	if (fpw == NULL) {
		couldntOpen();
		return;
	}

	fwrite(header.name, 2, 1, fpw);
	fwrite(&header.size, 3 * sizeof(int), 1, fpw);

	fwrite(&dibheader, sizeof(struct DIB_header), 1, fpw);

	for (int i = 0; i < pic.height; i++) { // Grayscaling the pixels
		for (int j = 0; j < pic.width; j++) {
			pic.rgb[i][j].red = pic.rgb[i][j].green = pic.rgb[i][j].blue = grayscale(pic.rgb[i][j]);
		}
	}

	for (int i = pic.height - 1; i >= 0; i--) { // writing them row by row
		fwrite(pic.rgb[i], pic.width, sizeof(struct RGB), fpw);
	}



	fclose(fpw);

	return;
}

void brightenRGB(struct BITMAP_header header, struct DIB_header dibheader, struct Image pic, int brightness) {
	FILE* fpw = fopen(tmpPath, "wb");
	if (fpw == NULL) {
		couldntOpen();
		return;
	}

	fwrite(header.name, 2, 1, fpw);
	fwrite(&header.size, 3 * sizeof(int), 1, fpw);

	fwrite(&dibheader, sizeof(struct DIB_header), 1, fpw);

	for (int i = 0; i < pic.height; i++) { // Saturating the pixels
		for (int j = 0; j < pic.width; j++) {
			pic.rgb[i][j] = changeBrightness(pic.rgb[i][j], brightness);
		}
	}

	for (int i = pic.height - 1; i >= 0; i--) { // writing them row by row
		fwrite(pic.rgb[i], pic.width, sizeof(struct RGB), fpw);
	}



	fclose(fpw);
}


void createImage(struct BITMAP_header header, struct DIB_header dibheader, struct Image pic) {

	//Creates a flipped image 

	FILE* fpw = fopen(tmpPath, "wb");
	if (fpw == NULL) {
		couldntOpen();
		return;
	}

	fwrite(header.name, 2, 1, fpw);
	fwrite(&header.size, 3 * sizeof(int), 1, fpw);

	fwrite(&dibheader, sizeof(struct DIB_header), 1, fpw);

	int count = 0;
	for (int i = pic.height - 1; i >= 0; i--) {
		fwrite(pic.rgb[count], pic.width, sizeof(struct RGB), fpw);
		count++;
	}

	fclose(fpw);
}


int flipImage() {

	// Calls the flipping image func 

	createImage(main_header, main_dibheader, main_pic);
	return 0;
}

void flipImageVert(struct BITMAP_header header, struct DIB_header dibheader, struct Image pic) {

	// Flips the image vertically 

	FILE* fpw = fopen(tmpPath, "wb");
	if (fpw == NULL) {
		couldntOpen();
		return;
	}

	fwrite(header.name, 2, 1, fpw);
	fwrite(&header.size, 3 * sizeof(int), 1, fpw);

	fwrite(&dibheader, sizeof(struct DIB_header), 1, fpw);

	struct RGB pixel;

	for (int i = 0; i < pic.height; i++) { // rows
		for (int j = 0; j < pic.width / 2; j++) { // width

			pixel = pic.rgb[i][j];
			pic.rgb[i][j] = pic.rgb[i][pic.width - j - 1];
			pic.rgb[i][pic.width - j - 1] = pixel;
		}
	}


	for (int i = pic.height - 1; i >= 0; i--) {
		fwrite(pic.rgb[i], pic.width, sizeof(struct RGB), fpw);
	}

	fclose(fpw);


}

void writeImageToSave(TCHAR pathImage[MAX_PATH]) {
	FILE* fpw = fopen(pathImage, "wb");
	if (fpw == NULL) {
		return;
	}

	fwrite(main_header.name, 2, 1, fpw);
	fwrite(&main_header.size, 3 * sizeof(int), 1, fpw);

	fwrite(&main_dibheader, sizeof(struct DIB_header), 1, fpw);

	for (int i = main_pic.height - 1; i >= 0; i--) {
		fwrite(main_pic.rgb[i], main_pic.width, sizeof(struct RGB), fpw);
	}

	fclose(fpw);
}



int checkFileIntegrity(TCHAR pathName[MAX_PATH]) {
	FILE* fp = fopen(pathName, "rb"); // read binary
	if (fp == NULL) {
		couldntOpen();
		return 1;
	}

	struct BITMAP_header header;
	struct DIB_header dibheader;

	fread(header.name, 2, 1, fp); //BM
	readint(fp, &header.size, 4);
	readint(fp, &header.garbage, 4);
	readint(fp, &header.image_offset, 4);

	if ((header.name[0] != 'B') || (header.name[1] != 'M')) {
		fclose(fp);
		return 1;
	}

	readint(fp, &dibheader.header_size, 4);
	readint(fp, &dibheader.width, 4);
	readint(fp, &dibheader.height, 4);
	readint(fp, &dibheader.colorplanes, 2); // 1
	readint(fp, &dibheader.bitsperpixel, 2); // 32
	readint(fp, &dibheader.compression, 4); // 0
	readint(fp, &dibheader.image_size, 4);
	readint(fp, &dibheader.temp[0], 4);
	readint(fp, &dibheader.temp[1], 4);
	readint(fp, &dibheader.temp[2], 4);
	readint(fp, &dibheader.temp[3], 4);

	if ((dibheader.header_size != 40) || (dibheader.compression != 0) || (dibheader.bitsperpixel != 24)) {
		fclose(fp);
		pathName = NULL;
		return 1;
	}

	fclose(fp);

	return 0;
}

void openbmpfile(TCHAR pathName[MAX_PATH]) {
	FILE* fp = fopen(pathName, "rb"); // read binary

	struct BITMAP_header header;
	struct DIB_header dibheader;

	fread(header.name, 2, 1, fp); //BM
	readint(fp, &header.size, 4);
	readint(fp, &header.garbage, 4);
	readint(fp, &header.image_offset, 4);

	readint(fp, &dibheader.header_size, 4);
	readint(fp, &dibheader.width, 4);
	readint(fp, &dibheader.height, 4);
	readint(fp, &dibheader.colorplanes, 2);
	readint(fp, &dibheader.bitsperpixel, 2);
	readint(fp, &dibheader.compression, 4);
	readint(fp, &dibheader.image_size, 4);
	readint(fp, &dibheader.temp[0], 4);
	readint(fp, &dibheader.temp[1], 4);
	readint(fp, &dibheader.temp[2], 4);
	readint(fp, &dibheader.temp[3], 4);

	char* buf = malloc(dibheader.image_size);
	if (!buf) {
		return 0;
	}

	fread(buf, 1, dibheader.image_size, fp);

	fseek(fp, header.image_offset, SEEK_SET);

	freeImage(main_pic);

	struct Image image = readImage(fp, dibheader.height, dibheader.width);

	main_dibheader = dibheader;
	main_header = header;
	main_pic = image;

	fclose(fp);

	return 0;
}
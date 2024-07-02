#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv) {

	FILE* file = fopen(argv[1], "rb");
	if (file == NULL) {
		printf("error opening file\n");
		return -1;
	}

	uint8_t header[14];
	fread(header, 14, 1, file);


	for (uint32_t i = 0; i < 14; i++) {
		printf("%x, ", header[i]);
	}

	uint16_t header_field = header[0] << 8 | header[1];
	uint32_t file_size = header[5] << 24 | header[4] << 16 | header[3] << 8 | header[2];

	printf("\nfile size: %d\n", file_size);
	
	uint32_t data_offset = header[13] << 24 | header[12] << 16 | header[11] << 8 | header[10]; 

	printf("data offset: %x\n", data_offset);

	uint32_t bitmap_infoheader_size;
	fread(&bitmap_infoheader_size, 4, 1, file);
	printf("header size: %d",bitmap_infoheader_size);


	uint32_t size[2];
	fread(size, 4, 2, file);
	printf("image size: %d x %d\n", size[0], size[1]);

	fseek(file, 2, SEEK_CUR);

	uint16_t bpp;
	fread(&bpp, 2, 1, file);
	printf("bits per pixel: %d\n", bpp);


	uint32_t compression;
	fread(&compression, 4, 1, file);
	printf("compression: %d\n", compression);

	uint32_t row_pad = (size[0] * bpp / 8 + 3) & (~3);
	printf("row padded: %d | %x\n", row_pad, row_pad);

	uint8_t *data = malloc(row_pad * size[1] * 8);
	fseek(file, data_offset, SEEK_SET);
	fread(data, row_pad*size[1], 1, file);
	

	return 0;
}

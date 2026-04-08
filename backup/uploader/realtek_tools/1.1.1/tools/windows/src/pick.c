/*

Compile:

Windows:
mingw32-gcc.exe -o pick.exe pick.c -static

linux:
gcc -o pick tools/linux/src/pick.c -static

macsox:
gcc -o tools/macos/pick tools/macos/src/pick.c

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/types.h>

#define PATTERN_1 0x96969999
#define PATTERN_2 0xFC66CC3F

unsigned int image_header[8] = {0,0,0,0,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

int main(int argc, char* argv[]) {
	int arg_num = 5;
	unsigned int is_raw = 0;

	if ((argc > 5) && ((strcmp(argv[5], "boot") == 0) || (strcmp(argv[5], "raw") == 0))) {
		if (strcmp(argv[5], "raw") == 0) {
			printf("is_law = 1\n");
			is_raw = 1;
		}

		arg_num++;
	}

	if (argc != arg_num) {
		printf("Usage: pick.exe <start addr> <end addr> <input name> <output name> [options]\nOptions:\n\tboot		To distinguish Image1 and Image2. Image1 should add this option\n\traw		Remove bss section according to start addr and end addr\n");
		return -1;
	}

	unsigned char *buf;
	unsigned int start;
	unsigned int end;
	unsigned int base;
	char *inf = argv[3];
	char *outf = argv[4];
	int size;
	FILE *ifp, *ofp;
	start = strtol(argv[1], NULL, 0);
	end = strtol(argv[2], NULL, 0);
	base = start&0xFFFF0000;
	printf("start = %x, end = %x, base = %x\n", start, end, base);

	ifp = fopen(inf, "rb");
	if (!ifp) {
		printf("Input file open error.");
		return -2;
	}
	ofp = fopen(outf, "wb");
	if (!ofp) {
		printf("Output file open error.");
		return -3;
	}

	//fseek(ifp, 0, SEEK_END);
	//size = ftell(ifp);
	size = end-start;
	printf("Input file size: %d\n", size);

	buf = malloc(size);
	if (!buf) {
		return -4;
	}
	if (end==0) {
		end = base+size;
	}

	if ((end - start + 1) > 0) {
	//	fseek(ifp, start-base, SEEK_SET);
		fseek(ifp, 0, SEEK_SET);

		fread(buf, end-start, 1, ifp);

		if (is_raw == 0) {
			if ((argc > 5) && (strcmp(argv[5], "boot") == 0)) {
				image_header[0] = PATTERN_1;
				image_header[1] = PATTERN_2;
			} else {
				image_header[0] = 0x35393138;
				image_header[1] = 0x31313738;
			}
			image_header[2] = size;
			image_header[3] = start;
			fwrite(image_header, 8, sizeof(unsigned int), ofp);
		}
		fwrite(buf, end-start, 1, ofp);
	}
	printf("copy size %d\n", end-start);
	fclose(ifp);
	fclose(ofp);
	free(buf);

	return 0;
}

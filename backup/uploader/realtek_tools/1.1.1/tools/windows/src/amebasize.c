/*

Compile:

Windows:
mingw32-gcc.exe -o amebasize.exe tools\windows\src\amebasize.c -static

linux:
gcc -o amebasize tools/linux/src/amebasize.c -static

macsox:
gcc -o tools/macos/amebasize tools/macos/src/amebasize.c

*/

#include <stdio.h>

int main(int argc, char *argv[]) {

	int size;
	char filename[512];

	if (argc < 2) {
		return -1;
	}

	sprintf(filename, "%s\\km0_boot_all.bin", argv[1]);
	FILE* fp_bootloader_km0 = fopen(filename, "rb");
	if (fp_bootloader_km0 != NULL) {
		fseek(fp_bootloader_km0, 0L, SEEK_END);
		size = ftell(fp_bootloader_km0);
		fclose(fp_bootloader_km0);
		printf("bootloader_km0 %d\n", size);
	}

	sprintf(filename, "%s\\km4_boot_all.bin", argv[1]);
	FILE* fp_bootloader_km4 = fopen(filename, "rb");
	if (fp_bootloader_km4 != NULL) {
		fseek(fp_bootloader_km4, 0L, SEEK_END);
		size = ftell(fp_bootloader_km4);
		fclose(fp_bootloader_km4);
		printf("bootloader_km4 %d\n", size);
	}

	sprintf(filename, "%s\\km0_km4_image2.bin", argv[1]);
	FILE* fp_img2 = fopen(filename, "rb");
	if (fp_img2 != NULL) {
		fseek(fp_img2, 0L, SEEK_END);
		size = ftell(fp_img2);
		fclose(fp_img2);
		printf("image2 %d\n", size);
	}

	return 0;
}

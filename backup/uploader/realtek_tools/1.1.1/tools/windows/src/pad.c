/*

Compile:

Windows:
mingw32-gcc.exe -o pad.exe pad.c -static

linux:
gcc -o pad tools/linux/src/pad.c -static

macsox:
gcc -o tools/macos/pad tools/macos/src/pad.c

*/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	unsigned int ret;

	if (argc != 3) {
		printf("pad.exe: fatal error\n");
		printf("Usage: pad.exe <file> <align_size>\n");
		return -1;
	}

	unsigned int align_size = strtoul(argv[2], NULL, 0);

	FILE *ifp1 = fopen(argv[1], "rb+"); //Open a binary file for both reading and writing. (The file must exist.)
	if (ifp1 == NULL) {
		printf("pad.exe: Open file error\n");
		return -1;
	}

	unsigned int img_len, pad_size;
	unsigned char pad_c = 0xFF;

	ret = fseek(ifp1, 0, SEEK_END);
	if (ret != 0) {
		printf("fseek error:%d\n", ret);
		goto exit;
	}

	img_len = ftell(ifp1);
	pad_size = ((((img_len - 1) / align_size) + 1) * align_size) - img_len;

	printf("pad.exe, img_len: %x, pad_len: %x\n", img_len, pad_size);

	while (pad_size-- != 0) {
		fwrite(&pad_c, 1, 1, ifp1);
	}

exit:
	fclose(ifp1);
	return 0;
}

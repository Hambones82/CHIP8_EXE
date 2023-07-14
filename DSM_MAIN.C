#include <stdio.h>
#include "c8dsm.h"

unsigned char chip8ROM[4096];

int main(int argc, char** argv) {
	int fileSize = 0;
	int i = 0;
	FILE *romFile;
	char disasm[MAX_DSM_MSG];
	int op = 0;

	romFile = fopen(argv[1], "r");
	if(argc == 1) {
		printf("Must specify rom file.\n");
		exit(1);
	}
	else if(romFile == NULL) {
		printf("Cannot open file ");
		printf(argv[1]);
		printf("\n");
		exit(1);
	}
	else {
		printf("File ");
		printf(argv[1]);
		printf(" successfully found.\n");
	}
	fseek(romFile, 0, SEEK_END);
	fileSize = ftell(romFile);
	printf("File size is %d\n", fileSize);
	fseek(romFile, 0, SEEK_SET);
	fread(chip8ROM+200, 1, fileSize, romFile);
	for(i = 200; i < 200 + fileSize; i+=2) {
		op = (((int)(chip8ROM[i])) << 8) + (int)(chip8ROM[i+1]);
		printf("address %04d: %04X; ", i, op);
		disasmOut(op, disasm);
		printf("instruction: %-14s\n", disasm);
	}


	return 0;
}

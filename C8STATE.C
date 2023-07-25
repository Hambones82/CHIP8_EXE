/*#define DEBUG_LOAD_ROM*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "c8state.h"
#include "debug.h"

unsigned char hexDigits[80] = {
0xF0, 0x90, 0x90, 0x90, 0xF0,
0x20, 0x60, 0x20, 0x20, 0x70,
0xF0, 0x10, 0xF0, 0x80, 0xF0,
0xF0, 0x10, 0xF0, 0x10, 0xF0,
0x90, 0x90, 0xF0, 0x10, 0x10,
0xF0, 0x80, 0xF0, 0x10, 0xF0,
0xF0, 0x80, 0xF0, 0x90, 0xF0,
0xF0, 0x10, 0x20, 0x40, 0x40,
0xF0, 0x90, 0xF0, 0x90, 0xF0,
0xF0, 0x90, 0xF0, 0x10, 0xF0,
0xF0, 0x90, 0xF0, 0x90, 0x90,
0xE0, 0x90, 0xE0, 0x90, 0xE0,
0xF0, 0x80, 0x80, 0x80, 0xF0,
0xE0, 0x90, 0x90, 0x90, 0xE0,
0xF0, 0x80, 0xF0, 0x80, 0xF0,
0xF0, 0x80, 0xF0, 0x80, 0x80
};

/* returns -1 if file cannot be opened, file size otherwise */
int loadROMFile(struct c8State *state, char *file) {
	int fileSize = 0;
	int bytesRead = 0;
	FILE *romFile;

	romFile = fopen(file, "rb");

	if(romFile == NULL) {
		printf("Cannot open file %s\n", file);
		return -1;
	}
	else {
		printf("File %s successfully found.\n", file);
	}
	fseek(romFile, (long)0, SEEK_END);
	fileSize = ftell(romFile);
	fseek(romFile, (long)0, SEEK_SET);
	while(bytesRead < fileSize) {
		int newBytesRead = fread(&(state->mem[0x0200+bytesRead]),
			1,
			fileSize-bytesRead,
			romFile);
#ifdef DEBUG_LOAD_ROM
		printf("new bytes read: %d\n", newBytesRead);
#endif
		bytesRead += newBytesRead;
#ifdef DEBUG_LOAD_ROM
			printf("bytes read %d, file size %d\n", bytesRead, fileSize);
#endif
			if(feof(romFile)) printf("EOF found\n");
	}
#ifdef DEBUG_LOAD_ROM
	dLog("file loaded successfully, size %d, read %d bytes",
		fileSize, bytesRead);
#endif
	/*todo: error handling for failure to close file*/
	fclose(romFile);
	return fileSize;
}

struct c8State *initC8State(void) {
	int i = 0;
	struct c8State *retVal = (struct c8State *)malloc(sizeof(struct c8State));
	memcpy(retVal->mem, hexDigits, 80);
	for(i = 0; i < 16; i++) {
		retVal->v[i] = 0;
		retVal->stack[i] = 0;
	}
	for(i = 80; i < 4096; i++) {
		retVal->mem[i] = 0;
	}
	retVal->PC = 0x0200; /* ?? */
	retVal->I = 0;
	retVal->SP = 0;
	retVal->keyPressedReg = 0;
	return retVal;
}

int getOp(struct c8State *state) {
	return (((int)(state->mem[state->PC])) << 8) +
		(int)(state->mem[state->PC+1]);
}

int getOpAt(struct c8State *state, int address) {
	return (((int)(state->mem[address])) << 8) +
		(int)(state->mem[address + 1]);
}

void testInit(struct c8State *state) {
	int i = 0;
	for(i = 0; i < 16; i++) {
		state->v[i] = i;
	}

}

#define TEXTMODE

#include <graph.h>

#define SCREEN_X 64
#define SCREEN_Y 32

char far *textFrameBuffer = (char far *)(0xB8000000);

void initDrawing() {
#ifdef GRAPHMODE
	_setvideomode(_MRES16COLOR);
#endif
#ifdef TEXTMODE
//clear the screen???
#endif
}

void deInitDrawing() {
	_setvideomode(_DEFAULTMODE);
}



/*
int drawPixel(/*int x, int y*/ unsigned int offset, int color) {
#ifdef TEXTMODE
	{
		unsigned char currentPixel = textFrameBuffer[offset];

	}
#endif
#ifdef GRAPHMODE
	int c8Color;
	int currentScreenColor = _getpixel(x, y);
	if(currentScreenColor == _BLACK) {
		c8Color = (color == 1) ? _WHITE : _BLACK;
	}
	else {
		c8Color = (color == 0) ? _WHITE : _BLACK;
	}
	_setcolor(c8Color);
	_setpixel(x, y);
#endif
}
*/

int drawCH8(char *memoryBuffer, int x, int y, int numBytes, int I) {
	unsigned char currentByte;
	int i = 0, dot = 0;

	for(i = 0; i < numBytes; i++) {
		currentByte = memoryBuffer[I+i];
		for(dot = 0; dot < 8; dot++) {
			drawPixel(x+dot, y+i, (currentByte & 0xF0) >> 7);
			currentByte <<= 1;
		}
	}
	/*need to return the overwrite*/
	return 0;
}

void clearScreen() {
#ifdef GRAPHMODE
	int x = 0, y = 0;
	_setcolor(_BLACK);
	for(x; x < SCREEN_X; x++) {
		for(y = 0; y < SCREEN_Y; y++) {
		_setpixel(x, y);
		}
	}
#endif
#ifdef TEXTMODE
//todo: implement text mode clear screen

#endif
}

/*#define DEBUG_DRAW*/
#include "draw.h"
#include "debug.h"
#include "timer.h"
#include <graph.h>
#include <stdarg.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

unsigned char far *textFrameBuffer = (char far *)(0xB8000000);

unsigned char maskLut[8] = {
0xFF, 0xFC,
0x07, 0x04,
0x07, 0xFC,
0xFF, 0x04
};


char far *getTextFBPtr(int x, int y) {
	return (char far *)(0xB8000000 + (y * 160) + (x * 2));
}

void setAttribute(int x, int y, unsigned char attribute) {
	*(getTextFBPtr(x, y)+1) = attribute;
}

void drawAttributeRange(int x0, int length, int y, unsigned char attribute) {
	int x = 0;
	for(x; x < length; x++) {
		*(getTextFBPtr(x+x0, y)+1) = attribute;
	}
}

void drawCharAt(int x, int y, char c) {
	*(getTextFBPtr(x, y)) = c;
}

int printfAt(int x, int y, char *msg, ...) {
	va_list args;
	int numCharsPrinted;

	va_start(args, msg);
	_settextposition(y, x);/*for some reason this is y, x...*/
	numCharsPrinted = vprintf(msg, args);
	if(numCharsPrinted == -1 || numCharsPrinted > TEXT_X) {
		printf("too many chars printed - limited to screen width");
		exit(1);
	}
	va_end(args);
	return numCharsPrinted;
}

void clearOutsideDisplay(void) {
	int x = 0, y = 0;
	for(y=0; y < TEXT_Y; y++) {
		for(x=0; x < TEXT_X; x++) {
			if((y < VERT_OFFSET-1) || (y > VERT_OFFSET+SCREEN_Y/2)) {
				drawCharAt(x, y, ' ');
			}
			else if(x < HOR_OFFSET-1 || x > HOR_OFFSET+SCREEN_X) {
				drawCharAt(x, y, ' ');
			}
		}
	}
}

void drawBorder(void) {
	int x, y;
	drawCharAt(HOR_OFFSET-1, VERT_OFFSET-1, 0xC9);
	drawCharAt(HOR_OFFSET+SCREEN_X, VERT_OFFSET-1, 0xBB);
	drawCharAt(HOR_OFFSET-1, VERT_OFFSET+SCREEN_Y/2, 0xC8);
	drawCharAt(HOR_OFFSET+SCREEN_X, VERT_OFFSET+SCREEN_Y/2, 0xBC);
	for(x = HOR_OFFSET; x < HOR_OFFSET+SCREEN_X; x++) {
		drawCharAt(x, VERT_OFFSET-1, 0xCD);
		drawCharAt(x, VERT_OFFSET+SCREEN_Y/2, 0xCD);
	}
	for(y = VERT_OFFSET; y < VERT_OFFSET+SCREEN_Y/2; y++) {
		drawCharAt(HOR_OFFSET-1, y, 0xBA);
		drawCharAt(HOR_OFFSET+SCREEN_X, y, 0xBA);
	}

}
/*assumes we use a border*/
void GetAreaOffset(unsigned char area, struct screenPos *pos) {
	switch(area) {
		case AREA_RIGHT:
			if((HOR_OFFSET * 2 + SCREEN_X) >= TEXT_X) {
				pos->x = -1;
				pos->y = -1;
			}
			else {
				pos->x = HOR_OFFSET + SCREEN_X + 2;
				pos->y = VERT_OFFSET;
			}
		break;
		case AREA_TOP:
			if(VERT_OFFSET<=1) {
				pos->x = -1;
				pos->y = -1;
			}
			else {
				pos->x = 0;
				pos->y = 0;
			}
		break;
		case AREA_LEFT:
			if(HOR_OFFSET<=1) {
				pos->x = -1;
				pos->y = -1;
			}
			else {
				pos->x = 0;
				pos->y = VERT_OFFSET;
			}
		break;
		case AREA_BOTTOM:
			if((VERT_OFFSET + 2 + SCREEN_Y/2) >= TEXT_Y) {
				pos->x = -1;
				pos->y = -1;
			}
			else {
				pos->x = 0;
				pos->y = VERT_OFFSET + 2 + SCREEN_Y / 2;
			}
		break;
		default:
			printf("trying to get area offset for invalid area");
			exit(1);
		break;
	}
}

void clearDisplay(void) {
	int x = 0, y = 0;
	for(x; x < SCREEN_X; x++) {
		for(y=0; y<SCREEN_Y/2; y++) {
			textFrameBuffer[(y+VERT_OFFSET)*160+((x+HOR_OFFSET)<<1)] = 0x20;
		}
	}
}

void clearScreen(void) {
	clearDisplay();
	clearOutsideDisplay();
	drawBorder();
}

void cursorOff(void) {
	union REGS regs;
	regs.h.ah = 1;
	regs.h.ch = 32;

	int86(0x10, &regs, &regs);
}

void cursorOn(void) {
	union REGS regs;
	regs.h.ah = 1;
	regs.h.ch = 0;
	regs.h.cl = 7;
	int86(0x10, &regs, &regs);
}

void initDrawing(void) {
	cursorOff();
	clearScreen();
}

void deInitDrawing(void) {
	cursorOn();
	_setvideomode(_DEFAULTMODE);

}



int drawCH8(char *memoryBuffer, int x, int y, int numBytes, int I) {
	int retVal = 0; /* 0 means no collision, 1 means collision*/
/*
//we need a translation of y to address offset.
//in the text buffer, even addresses are reserved for ascii codes and odds
//are for attributes.  there are 80 columns per row, so 80 x 2 = 160 bytes
//per row.  however, we are using half-blocks for rows, so two rows will
//sit on the same "memory row".  in this case, even y values will be the top
//half block and odd y values will be the bottom half block.
//there will be four possible ascii values based on the state.  here are the
//possible values and transitions:
//20 0010'0000: (space)
	//(0) to top block: flip all bits             -- mask: 0xFF
	//(1) to bottom block: flip bits 7 through 2  -- mask: 0xFC
//DB 1101'1011: (full block)
	//(6) to top block: flip bit 2                -- mask: 0x04
	//(7) to bottom block: flip bits 2, 1, and 0  -- mask: 0x07
//DC 1101'1100: (bottom block)
	//(4) to full block: flip bits 2, 1, and 0    -- mask: 0x07
	//(5) to space: flip bits 7-2                 -- mask: 0xFC
//DF 1101'1111: (top block)
	//(2) to space: flip all                      -- mask: 0xFF
	//(3) to full block: flip bit 2               -- mask: 0x04
//possible inputs:
	//row even or odd (y & 1) -> selects operating on top or bottom
		//selects even transitions for even, odd transitions for odd
			//thus could easily just OR the parity bit with the LUT
			//select bit
	//current color: (one of the asciis above)
		//this is what gets the mask applied to it
		//also selects a mask using bits 2 and 1.
		//color & 0x06 >> 1
	//input color
		//selects either FF or 00 to AND with mask
//not getting all of the bits...
*/
int c8RowOffset = 0;
int c8Col = x&0x3F;
int memRowOffset = (((c8RowOffset + y) >> 1) + VERT_OFFSET) * 160;
unsigned char clipMask;
unsigned char clipShift = 64 - (unsigned char)c8Col;
unsigned char clipVert = 32 - (y&0x1F);
unsigned char numLines = numBytes > clipVert ? clipVert : numBytes;
//setDrawHalt();
if(c8Col < 56) clipMask = 0xFF;
else clipMask = ((1 << clipShift) - 1) << (8 - clipShift);
for(c8RowOffset; c8RowOffset < numLines; c8RowOffset++) {
	int i = 0;
	unsigned char c8Byte = memoryBuffer[I+c8RowOffset] & clipMask;
	memRowOffset = ((((c8RowOffset + y)&0x1F) >> 1) + VERT_OFFSET) * 160;
	/*for(c8Col = x; c8Col < x + 8; c8Col++) {*/
	for(i = 0; i < 8; i++) {
		c8Col = (x + i) & 0x003F;
		{
		unsigned char fb_char = textFrameBuffer[memRowOffset +
			((c8Col+HOR_OFFSET)<<1)];
		char c8Color = (c8Byte & 0x80) ? 0xFF : 0;
		unsigned char parity = (c8RowOffset + y) & 1;
		unsigned char maskSelect = parity | (fb_char & 0x06);
		unsigned char newFBChar = fb_char ^ (maskLut[maskSelect] & c8Color);
		textFrameBuffer[memRowOffset + ((c8Col+HOR_OFFSET)<<1)] = newFBChar;
		c8Byte <<= 1;
		retVal |= c8Color && ((maskSelect == 2) || (maskSelect == 3) ||
			(maskSelect == 5) || (maskSelect == 6));
		}
	/*
	DRAW_CH8_INNER
	c8Col++;
	DRAW_CH8_INNER
	c8Col++;
	DRAW_CH8_INNER
	c8Col++;
	DRAW_CH8_INNER
	c8Col++;
	DRAW_CH8_INNER
	c8Col++;
	DRAW_CH8_INNER
	c8Col++;
	DRAW_CH8_INNER
	c8Col++;
	DRAW_CH8_INNER*/
	}
}

	/*need to return the overwrite*/
	return retVal;
}


#ifndef DRAW_H
#define DRAW_H

#define VERT_OFFSET 1
#define HOR_OFFSET 1

#define SCREEN_X 64
#define SCREEN_Y 32

#define TEXT_X 80
#define TEXT_Y 25

#define ATTR_BLINK      0x80
#define ATTR_BLACK_BG   0x00
#define ATTR_LIGHT_BG   0x70
#define ATTR_BOLD       0x08
#define ATTR_BLACK_TEXT 0x00
#define ATTR_WHITE_TEXT 0x07
#define ATTR_RED_TEXT   0x04

struct screenPos {
	int x;
	int y;
};

struct chip8FB {
	unsigned char bytes[256];
};

void initDrawing(void);
void deInitDrawing(void);

int drawCH8(char *memoryBuffer, int x, int y, int numBytes, int I);
void clearScreen(void);
char far *getTextFBPtr(int x, int y);
void setAttribute(int x, int y, unsigned char attribute);
void drawAttributeRange(int x0, int x1, int y, unsigned char attributes);
void drawCharAt(int x, int y, char c);
void clearDisplay(void);
int printfAt(int x, int y, char *msg, ...);
#define AREA_RIGHT  0
#define AREA_TOP    1
#define AREA_LEFT   2
#define AREA_BOTTOM 3
void GetAreaOffset(unsigned char area, struct screenPos *pos);
void serializeFB(unsigned char *output);

#endif

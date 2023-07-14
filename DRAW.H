#ifndef DRAW_H
#define DRAW_H

struct screenPos {
	int x;
	int y;
};

void initDrawing(void);
void deInitDrawing(void);

int drawCH8(char *memoryBuffer, int x, int y, int numBytes, int I);
void clearScreen(void);
char far *getTextFBPtr(int x, int y);
void drawCharAt(int x, int y, char c);
void clearDisplay(void);
void printfAt(int x, int y, char *msg, ...);
#define AREA_RIGHT  0
#define AREA_TOP    1
#define AREA_LEFT   2
#define AREA_BOTTOM 3
void GetAreaOffset(unsigned char area, struct screenPos *pos);

#endif

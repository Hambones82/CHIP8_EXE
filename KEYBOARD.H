#ifndef KEYBOARD_H
#define KEYBOARD_H

void startKeyboard(void);
void endKeyboard(void);
unsigned char getScanCodeDown(unsigned char scanCode);
unsigned char getExtendedScanCodeDown(unsigned char scanCode);
int isRecentKeyAvailable(void);
unsigned char getRecentKeyPressed(void);
void clearRecentKeyAvailable(void);

#endif

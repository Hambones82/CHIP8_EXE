/*code taken from stack overflow 'checking if a key is down in MS-DOS*/

#include <conio.h>
#include <dos.h>
#include <stdio.h>

static void (interrupt far *old_keyb_int)(void);

unsigned char normal_keys[0x60];
unsigned char extended_keys[0x60];
/*
//this assumes that max only one keypress will occur in between calls
//to getrecentkeypressed
*/
unsigned char recentKeyPressed;
unsigned char recentKeyAvailable;

static void interrupt far keyb_int(void) {
	unsigned char port_b;
	unsigned char rawcode;

	static unsigned char buffer;
	unsigned char make_break;
	int scancode;
	_disable();
	rawcode = inp(0x60);

	make_break = !(rawcode & 0x80);
	scancode = rawcode & 0x7F;

	if(buffer == 0xE0) {
		if(scancode < 0x60) {
			extended_keys[scancode] = make_break;
		}
		buffer = 0;
	} else if (buffer >= 0xE1 && buffer <= 0xE2) {
		buffer = 0;
	} else if (rawcode >= 0xE0 && rawcode <= 0xE2) {
		buffer = rawcode;
	} else if (scancode < 0x60) {
		if((normal_keys[scancode] == 0) && (make_break == 1)) {
			recentKeyAvailable = 1;
			recentKeyPressed = scancode;
		}
		normal_keys[scancode] = make_break;
	}

	/*clear key latch*/
	port_b = inp(0x61);
	port_b |= 0x80;
	outp(0x61, port_b);
	port_b = inp(0x61);
	port_b &= 0x7F;
	outp(0x61, port_b);
	/*end interrupts*/
	outp(0x20, 0x20);
	_enable();
}

void startKeyboard(void) {
	old_keyb_int = (void (interrupt far *)())(_dos_getvect(0x09));
	_dos_setvect(0x09, keyb_int);
	recentKeyAvailable = 0;
}
/*1 if yes, 0 if no*/
int isRecentKeyAvailable(void) {
	return recentKeyAvailable;
}

/*must be called at the end of the main loop*/
void clearRecentKeyAvailable(void) {
	recentKeyAvailable = 0;
}


unsigned char getRecentKeyPressed(void) {
	clearRecentKeyAvailable();
	return recentKeyPressed;
}

void endKeyboard(void) {
	if(old_keyb_int != NULL) {
		_dos_setvect(0x09, old_keyb_int);
		old_keyb_int = NULL;
	}
}

unsigned char getScanCodeDown(unsigned char scanCode) {
	return normal_keys[scanCode];
}

unsigned char getExtendedScanCodeDown(unsigned char scanCode) {
	return extended_keys[scanCode];
}



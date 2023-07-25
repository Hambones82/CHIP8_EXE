#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include "sound.h"
#include "debug.h"
#include "timer.h"

/*should have a way to just register functions and call them as necessary...
*/
TimerCallback tCallbacks[MAX_TIMER_CALLBACKS];
int tCBCounter = 0;

unsigned char haltAfterDraw = 0;
unsigned char timer;
unsigned char soundTimer;

void RegisterTimerCallback(TimerCallback cb) {
	if(tCBCounter == MAX_TIMER_CALLBACKS) {
		printf("too many timer callbacks registered\n");
		exit(1);
	}
	else {
		tCallbacks[tCBCounter++] = cb;
	}
}

static void interrupt far timer_handler(void) {

	int i = 0;
	for(i; i < tCBCounter; i++) {
		tCallbacks[i]();
	}

	//_disable();
	resetDrawHalt();
	if(timer) {
		timer--;
	}
	if(soundTimer) {
		//dLog("sound timer val: %d\n", soundTimer);
		soundTimer--;
		if(soundTimer == 0) {
			//dLog("ending beep\n");
			stopBeep();
		}
	}
	//_enable();
}

void (interrupt far *old_timer)();

void startTimer(void) {
	char escape_char = 0;
	_disable();
	old_timer = _dos_getvect(0x1C);
	outp(0x43, 0x36);
	escape_char = 1;
	outp(0x40, 0xAE);
	escape_char = 2;
	outp(0x40, 0x4D);

	_dos_setvect(0x1C, timer_handler);
	_enable();
}

void endTimer(void) {
	_dos_setvect(0x1C, old_timer);
}

void setTimer(unsigned char val) {
	timer = val;
}

unsigned char getTimer(void) {
	return timer;
}

void setSoundTimer(unsigned char val) {
	soundTimer = val;
}

unsigned char getSoundTimer(void) {
	return soundTimer;
}

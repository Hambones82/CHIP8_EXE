#include <conio.h>

unsigned int note = 4560;

void initSound(void) {
	outp(0x43, 0xb6);
	outp(0x42, note & 0x00FF);
	outp(0x42, note >> 8);
}

void startBeep(void) {
	unsigned char p61_data = inp(0x61);
	p61_data |= 0x03;
	outp(0x61, p61_data);
}

/*add the pragma*/
#pragma check_stack(off)
void stopBeep(void) {
	unsigned char p61_data = inp(0x61);
	p61_data &= 0xFC;
	outp(0x61, p61_data);
}

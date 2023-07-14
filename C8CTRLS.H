#include "c8state.h"

#ifndef C8CTRLS_H
#define C8CTRLS_H

#define C8_NO_HALT          0
#define C8_STEP_HALT        1
#define C8_DRAW_HALT        2
#define C8_FX0A_HALT        3

#define C8SETTING_ON        1
#define C8SETTING_OFF       0

struct c8Controls {
	unsigned char draw_halted;
	unsigned char step_halted;
	unsigned char FX0A_halted;
};

struct c8Setting {
	unsigned char settingType;
	unsigned char onOff;
};

void initControls(void);
void updateCtrls(struct c8Setting *setting);
void c8CtrlExecute(struct c8State *state);

#endif

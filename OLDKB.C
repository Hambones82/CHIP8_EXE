/*code taken from stack overflow 'checking if a key is down in MS-DOS*/

#include <conio.h>
#include <dos.h>
#include <stdio.h>


static void (interrupt far *old_keyb_int)();

int make_break;
union REGS input_regs, output_regs;
unsigned char normal_keys[0x60];
unsigned char extended_keys[0x60];
unsigned char count[2] = {0,0};

static void interrupt far keyb_int() {
	unsigned char port_b;
	unsigned char rawcode = inp(0x60);
	//count[0] += 1;
	make_break = rawcode & 0x80;
	if(make_break) * ((char far *)(0xB8000000)) = 'y';
	else * ((char far *)(0xB8000000)) = 'n';
	//old_keyb_int();
	port_b = inp(0x61);
	port_b |= 0x80;
	outp(0x61, port_b);
	port_b = inp(0x61);
	port_b &= 0x7F;
	outp(0x61, port_b);

	outp(0x20, 0x20);
}
/*
	static unsigned char buffer;
	unsigned char rawcode;
	unsigned char make_break;
	int scancode;
	int keypressed = 0;

	rawcode = inp(0x60);
	make_break = !(rawcode & 0x80);
	scancode = rawcode & 0x7F;

	keypressed = make_break;


	if(rawcode & 0x80) {
		* ((char far *)(0xB8000000)) = 'y';
	}
	else {
		* ((char far *)(0xB8000000)) = 'l';
	}


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
		normal_keys[scancode] = make_break;
	}
*/

//}

void startKeyboard(void) {
	old_keyb_int = (void (interrupt far *)())(_dos_getvect(0x09));
	_dos_setvect(0x09, keyb_int);
}

void endKeyboard(void) {
	if(old_keyb_int != NULL) {
		_dos_setvect(0x09, old_keyb_int);
		old_keyb_int = NULL;
	}
}

int ctrlbrk_handler(void) {
	unhook_keyb_int();
	//_setcursortype(_NORMALCURSOR);
	return 0;
}

static putkeys(int y, unsigned char const *keys) {
	int i;
	//gotoxy(1, y);
	for(i = 0; i < 0x30; i++) {
		putch(keys[i] + '0');
	}
}

void game(void) {
	//_setcursortype(_NOCURSOR);
	//clrscr();
	while(!normal_keys[1]) {
		putkeys(1, normal_keys);
		putkeys(2, normal_keys + 0x30);
		putkeys(4, extended_keys);
		putkeys(5, extended_keys + 0x30);
	}
	//gotoxy(1,6);
	//_setcursortype(_NORMALCURSOR);
}

int main() {
	//ctrlbrk(ctrlbrk_handler);
	int quit = 0;
	startKeyboard();
	printf("hooked\n");
	while(!quit) {
	/*
	   if(normal_keys[0x1e])
			*((char far *)(0xb8000000)) = 'y';
	   else
			*((char far *)(0xb8000000)) = 'n';
	*/
	   //if(keypressed) quit = 1;
	   //printf("%d", count[0]);

	   //if(kbhit()) {
	   //     printf("keyboard hit");
	   //     getch();
	   //
		input_regs.h.ah = 0;
		int86(0x16, &input_regs, &output_regs);
	   //}
	}

	endKeyboard();
	return 0;
}



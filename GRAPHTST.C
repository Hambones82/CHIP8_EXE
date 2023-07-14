#include <stdio.h>

void main() {
	__asm {
		mov ax, 21h;
		int 21;
	}
}

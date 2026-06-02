#ifndef _KERNEL_KEYBOARD_H
#define _KERNEL_KEYBOARD_H

void keyboard_initialize();
void keyboard_poll();

// 1 - OK, 0 - None
int keyboard_getchar(char* out);

#endif

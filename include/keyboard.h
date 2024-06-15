#ifndef __KEYBOARD_H
#define __KEYBOARD_H
#define KBD_DATA_PORT   0x60
#define KBD_CTRL_PORT   0x64
#define CON(A) (A-'@')
#define ESC_ASCII 0x1B

#define EXTEND_MODE (1<<0)
#define SHIFT_MODE (1<<1)
#define CTRL_MODE (1<<2)
#define ALT_MODE (1<<3)
#define CAPSLOCK  (1<<4)
#define NUMLOCK  (1<<5)
#define SCROLLLOCK (1<<6)

#define LSHIFT_MAKE 0x2a
#define RSHIFT_MAKE 0x36
#define LSHIFT_BREAK 0xaa
#define RSHIFT_BREAK 0xb6

#define LALT_MAKE 0x38
#define RALT_MAKE 0xe038
#define LALT_BREAK 0xb8
#define RALT_BREAK 0xe0b8

#define LCTRL_MAKE 0x1d
#define RCTRL_MAKE 0xe01d
#define LCTRL_BREAK 0x9d
#define RCTRL_BREAK 0xe09d

#define CAPSLOCK_MAKE 0x3a
#define CAPSLOCK_BREAK 0xba

#define NUMLOCK_MAKE 0x45
#define NUMLOCK_BREAK 0xc5

#endif
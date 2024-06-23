#include "keyboard.h"
#include "types.h"
#include "io.h"
#include "print.h"
//分别将扫描码映射为正常状态，按下shift状态，按下ctrl状态
static unsigned char keymap[256][3] = {

    /*0-0x3*/       {0,   0,   0},   {ESC_ASCII,   ESC_ASCII,   0},   {'1',   '!',   0},   {'2',   '@',   0},
    /*0x4-0x7*/     {'3',   '#',   0},    {'4',   '$',   0},   {'5',   '%',   0},   {'6',   '^',   0},
    /*0x8-0xB*/     {'7',   '&',   0},   {'8',   '*',   0},   {'9',   '(',   0},   {'0',   ')',   0},
    /*0xC-0xF*/     {'-',   '_',   0},   {'=',   '+',   0},   {'\b',   '\b',   0},   {'\t',   '\t',   0},
    /*0x10-0x13*/   {'q',   'Q',   CON('Q')},   {'w',   'W',   CON('W')},   {'e',   'E',   CON('E')},   {'r',   'R',   CON('R')},
    /*0x14-0x17*/   {'t',   'T',   CON('T')},   {'y',   'Y',   CON('Y')},   {'u',   'U',   CON('U')},   {'i',   'I',   CON('I')},
    /*0x18-0x1B*/   {'o',   'O',   CON('O')},   {'p',   'P',   CON('P')},   {'[',   '{',   0},   {']',   '}',   0},
    /*0x1C-0x1F*/   {'\n',   '\n',   '\r'},   {0,   0,   0},   {'a',   'A',   CON('A')},   {'s',   'S',   CON('S')},
    /*0x20-0x23*/   {'d',   'D',   CON('D')},   {'f',   'F',   CON('F')},   {'g',   'G',   CON('G')},   {'h',   'H',   CON('H')},
    /*0x24-0x27*/   {'j',   'J',   CON('J')},   {'k',   'K',   CON('K')},   {'l',   'L',   0},   {';',   ':',   0},
    /*0x28-0x2B*/   {'\'',   '"',   0},   {'`',   '~',   0},   {0,   0,   0},   { '\\',   '|',   0 },
    /*0x2C-0x2F*/   {'z',   'Z',   CON('Z')},      {'x',   'X',   CON('X')},   {'c',   'C',   CON('C')},   {'v',   'V',   CON('V')},
    /*0x30-0x33*/   {'b',   'B',   CON('B')},   {'n',   'N',   CON('N')},   {'m',   'M',   CON('M')},   {',',   '<',   0},
    /*0x34-0x37*/   {'.',   '>',   0},   {'/',   '?',   0},   {0,   0,   0},   {'*',   '*',   0},
    /*0x38-0x3B*/   {0,   0,   0},   {' ',   ' ',   0},   {0,   0,   0},   {0,   0,   0},
    /*0x3C-0x3F*/   {0,   0,   0},   {0,   0,   0},   {0,   0,   0},   {0,   0,   0},
    /*0x40-0x43*/   {0,   0,   0},   {0,   0,   0},   {0,   0,   0},   {0,   0,   0},
    /*0x44-0x47*/   {0,   0,   0},   {0,   0,   0},   {0,   0,   0},   {'7',   '7',   0},
    /*0x48-0x4B*/   {'8',   '8',   0},   {'9',   '9',   0},   {'-',   '-',   0},   {'4',   '4',   0},
    /*0x4C-0x4F*/   {'5',   '5',   0},   {'6',   '6',   0},   {'+',   '+',   0},   {'1',   '1',   0},
    /*0x50-0x54*/   {'2',   '2',   0},   {'3',   '3',   0},   {'0',   '0',   0},   {'.',   '.',   0}
};
char get_kbdchar() {
    static uint8_t mode;
    uint8_t status = inb(KBD_CTRL_PORT);
    uint16_t full_code;
    //是否有数据
    if (!(status & 1)) {
        return 0;
    }
    uint8_t data = inb(KBD_DATA_PORT);
    full_code = data;

    //扩展码前缀
    if (data == 0xE0) {
        mode |= EXTEND_MODE;
        return 0;
    }
    
    //上一次是否为扩展码前缀，若是，则此次为数据，并清除扩展状态
    if (mode & EXTEND_MODE) {
        full_code |= 0xe000;
        mode &= ~EXTEND_MODE;
    }

    //断码
    if (data & 0x80) {
        //是否为状态键松开
        switch (full_code) {
        case LSHIFT_BREAK:
        case RSHIFT_BREAK:
            mode &= ~(SHIFT_MODE | EXTEND_MODE);
            return 0;
        case LCTRL_BREAK:
        case RCTRL_BREAK:
            mode &= ~(CTRL_MODE | EXTEND_MODE);
            return 0;
        case LALT_BREAK:
        case RALT_BREAK:
            mode &= ~(ALT_MODE | EXTEND_MODE);
            return 0;
        case CAPSLOCK_BREAK:
            //mode &= ^ (CAPSLOCK);
            mode &= ~EXTEND_MODE;
            return 0;
        case NUMLOCK_BREAK:
            //mode &= ^NUMLOCK;
            mode &= ~EXTEND_MODE;
            return 0;
        default:
            return 0;
        }
        return 0;
    }
    //通码
    else {
        //是否为状态键按下
        switch (full_code) {
        case LSHIFT_MAKE:
        case RSHIFT_MAKE:
            mode |= SHIFT_MODE;
            mode &= ~EXTEND_MODE;
            return 0;
        case LCTRL_MAKE:
        case RCTRL_MAKE:
            mode |= CTRL_MODE;
            mode &= ~EXTEND_MODE;
            return 0;
        case LALT_MAKE:
        case RALT_MAKE:
            mode |= ALT_MODE;
            mode &= ~EXTEND_MODE;
            return 0;
        case CAPSLOCK_MAKE:
            mode ^= CAPSLOCK;
            mode &= ~EXTEND_MODE;
            return 0;
        case NUMLOCK_MAKE:
            mode ^= NUMLOCK;
            mode &= ~EXTEND_MODE;
            return 0;
        }
        //不是状态键按下
        char ch = 0;
        if (!mode) {
            ch = keymap[data][0];
        }
        
        else if (mode & SHIFT_MODE) {
            if (mode & CAPSLOCK) {
                ch = keymap[data][0];
            }
            else {
                ch = keymap[data][1];
            }
        }
        else if (mode & CAPSLOCK) {
            ch = keymap[data][1];
        }
        else if (mode & CTRL_MODE) {
            ch = keymap[data][2];
        }
        else {
            ch = keymap[data][0];
        }
        return ch;
    }

}
void do_keyboard() {
    char ch = get_kbdchar();
    if (ch) {
        put_char(ch);
    }  
}
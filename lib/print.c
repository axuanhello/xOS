#include "print.h"
#include "types.h"
#include "io.h"
#include "string.h"
/** 只使用第一页的显存。即物理地址范围：0xb8000~0xb8000+80*25*2=0xb8fa0
*   通过将字符复制到上一行实现滚屏操作。
*   TODO:特权级切换，以及虚拟内存下的显存地址
*/

uint16_t get_cursor(void) {
    uint16_t result = 0;
    //获取光标低8位。端口0x3d4用于设置索引寄存器
    outb(0x3d4, 0xf);
    result = inb(0x3d5);
    //获取光标高8位
    outb(0x3d4, 0xe);
    result += ((uint16_t)inb(0x3d5)) << 8;  //bug.不是>>，而是<<...
    return result;
}

inline uint8_t get_cursor_r(void) {
    return (uint8_t) (get_cursor() / VGA_WIDTH);
}
inline uint8_t get_cursor_c(void) {
    return (uint8_t) (get_cursor() % VGA_WIDTH);
}

void set_cursor(uint16_t cursor) {   
    /*不要自动翻页，无法决定是否回到行首。由输出函数决定。
    if (cursor >= 80 * 25) {
        roll_up();
        cursor = 80*24;
    }*/
    cursor %= VGA_WIDTH * VGA_HEIGHT;
    //设置光标低8位
    outb(0x3d4, 0xf);
    outb(0x3d5, (uint8_t)cursor);

    //设置光标高8位
    outb(0x3d4, 0xe);
    outb(0x3d5, (uint8_t)(cursor >> 8));
}

void set_cursor_r_c(uint8_t row, uint8_t column) {
    set_cursor(row*VGA_WIDTH+column);
}
//在光标对应显存处写入数据
static inline void write_video(const uint16_t cursor,const uint8_t val) {
    *(char*)(0xb8000 + cursor * 2) = val;
}

//屏幕上移一行
void roll_up() {
    for (int row = 1;row < VGA_HEIGHT;++row) {
        memcpy((void*)((row - 1) * VGA_WIDTH*2 + 0xb8000), (void*)((row)*VGA_WIDTH*2 + 0xb8000), VGA_WIDTH*2);//每个字符显示要2字节，莫忘*2
    }
    uint16_t cursor = (VGA_HEIGHT - 1) * VGA_WIDTH;
    for (int i = 0;i < VGA_WIDTH;++i) {
        write_video(++cursor,0);
    }
}

void put_char(const char ch) {
    uint16_t cursor = get_cursor();
    switch (ch) {
    //此处'\n'效果默认等价于'\n\r'
    case '\n':
        if (cursor / VGA_WIDTH +1 == VGA_HEIGHT) {
            roll_up();
            set_cursor_r_c(VGA_HEIGHT-1,0);
        }
        else {
            set_cursor_r_c(cursor/VGA_WIDTH+1,0);
        }
        break;
    case '\r':
        set_cursor(cursor / VGA_WIDTH * VGA_WIDTH);
        break;
    //删除当前光标处内容，然后将光标向前移动一格。
    case '\b':
        write_video(cursor, 0);
        if (cursor != 0) {
            set_cursor(cursor - 1);
        }
        break;
    //制表符采用4个格子。
    case '\t':
        set_cursor((cursor / __TAB_SIZE+1) * __TAB_SIZE);
        break;
    case '\v':
        set_cursor(cursor + VGA_WIDTH);
        break;
    //默认字符则在光标所在处写入内容，然后将光标向后移动。如果光标向后移动超过范围，则翻页。 
    default:
        write_video(cursor, ch);
        if ((++cursor) == VGA_WIDTH * VGA_HEIGHT) {
            roll_up();
            set_cursor_r_c(VGA_HEIGHT-1,0);
        }
        else {
            set_cursor(cursor);
        }
        break;
    }
}

void print(const char* str) {
    size_t len = strlen(str);
    for (size_t i = 0;i < len;++i) {
        put_char(str[i]);
    }
}

void put_int(int num) {
    char show[10] = { 0 };//int最多10位十进制表示
    if (num < 0) {
        put_char('-');
        num = -num;
    }
    int i = sizeof show;
    do {
        show[--i] = (num % 10 + 48);    //0 ASCII:48    
    } while ((num /= 10) && i);
    //逆序输出；从有数据开始输出
    for (int j = 0;j < sizeof show;++j) {
        if (show[j]) {
            put_char(show[j]);
        }    
    }
}
void put_uint(unsigned int num) {
    char show[10] = { 0 };
    int i = 10;
    do {
        show[--i] = (num % 10 + 48);    //0 ASCII:48    
    } while ((num /= 10) && i);
    //逆序输出；从有数据开始输出
    for (int j = 0;j < sizeof show;++j) {
        if (show[j]) {
            put_char(show[j]);
        }    
    }
}
void clear_screen() {
    for (int i = 0;i < VGA_WIDTH * VGA_HEIGHT;++i) {
        write_video(i, 0);
    }
}
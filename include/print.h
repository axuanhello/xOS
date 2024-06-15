//Note: 尚未处理虚拟地址，特权级相关问题。
#ifndef __PRINT_H
#define __PRINT_H

#include"types.h"

#ifndef __TAB_SIZE
#define __TAB_SIZE 4
#endif

#define VGA_HEIGHT 25
#define VGA_WIDTH 80

uint16_t get_cursor(void);
uint8_t get_cursor_r(void);
uint8_t get_cursor_c(void);
void set_cursor(uint16_t cursor);
void set_cursor_r_c(uint8_t row, uint8_t column);
//void roll_up();
void put_char(const char ch);
void put_int(int num);
void put_uint(unsigned int num);
void put_uinth(unsigned int num);
void put_str(const char* str);
void clear_screen();

#endif
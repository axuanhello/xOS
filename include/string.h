#ifndef __STRING_H
#define __STRING_H

#ifndef NULL
#define NULL (void*(0))
#endif
#include"types.h"
size_t strlen(const char* str);
//若count输入负数将会导致复制次数错误。
void* memcpy(void* dest, const void* src, size_t count);
#endif
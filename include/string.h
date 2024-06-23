#ifndef __STRING_H
#define __STRING_H

#ifndef NULL
#define NULL ((void*)(0))
#endif
#include"types.h"
size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
char* strcat(char* dest, const char* src);
size_t strspn(const char* str, const char* strSet);
size_t strcspn(const char* str, const char* strSet);
char* strchr(const char* s, char c);
char* strpbrk(const char* str, const char* strSet);
char* strtok(char* str, const char* delim);
char* strtok_s(char* str, const char* delim, char** context);
//若count输入负数将会导致复制次数错误。
void* memcpy(void* dest, const void* src, size_t count);
void* memset(void* str, int c, size_t n);
int memcmp(const void* str1, const void* str2, size_t n);
#endif
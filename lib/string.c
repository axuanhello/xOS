#include"types.h"
#include"string.h"
/*
默认使用%ds:%edi
inline int strlen(const char* str) {
    int result = 0;
    asm("cld;\n\t"
        "repne scasb;\n\t"
        "notl %%ecx;\n\t"
        "decl %%ecx;"
        :"=c"(result)
        :"+a"(0),"D"(str),"c"(0xffffffff));//ecx初始赋值最大，循环一只减，最后取反，可得循环次数
    return result;
}
*/
size_t strlen(const char* str) {
    size_t result = 0;
    while (*str != 0) {
        ++str;
        ++result;
    }
    return result;
}

int strcmp(const char* str1, const char* str2) {
    size_t i = 0;
    while (str1[i] && (str1[i] == str2[i])) { ++i; }
    return str1[i] - str2[i];
}

char* strcat(char* dest, const char* src) {
    while (*dest++);
    --dest;
    while (*dest++ = *src++);
    
}

void* memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (char*)src;
    while (count--) {
        *d = *s;
        ++d;
        ++s;
    }
    return dest;
}

void* memset(void* str, int c, size_t n) {
    char* s = (char*)str;
    while (n--) {
        *s++ = c;
    }
    return str;
}

int memcmp(const void* str1, const void* str2, size_t n) {
    const char* s1 = (char*)str1;
    const char* s2 = (char*)str2;
    for (size_t i = 0;i < n;++i) {
        if (s1[i] == s2[i]) {
            continue;
        }
        else if (s1[i] < s2[i]) {
            return -1;
        }
        else if (s1[i] > s2[i]) {
            return 1;
        }
    }
    return 0;
}

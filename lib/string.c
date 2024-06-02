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
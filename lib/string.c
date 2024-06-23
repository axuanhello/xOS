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
//返回从头开始的子串长度，这一段子串中的字符均可以在strSet中找到。
size_t strspn(const char* str, const char* strSet) {
    const char* p1, * p2;
    size_t count = 0;
    for (p1 = str; *p1 != '\0'; ++p1) {
        for (p2 = strSet; *p2 != '\0'; ++p2) {
            if (*p1 == *p2) {
                break;
            }
        }
        if (*p2 == '\0') {
            return count;
        }
        ++count;
    }
    return count;
}
//返回从头开始的子串长度，这一段子串中的字符均不能在strSet中找到。
size_t strcspn(const char* str, const char* strSet) {
    const char* p1, * p2;
    size_t count = 0;
    for (p1 = str; *p1 != '\0'; ++p1) {
        for (p2 = strSet; *p2 != '\0'; ++p2) {
            if (*p1 == *p2)
                return count;
        }
        ++count;
    }

    return count;
}
//寻找第一个出现c的位置
char* strchr(const char* s, char c) {
    for (;*s != c; ++s)
        if (*s == '\0') {
            return NULL;
        }
    return (char*)s;
}
//返回str中第一个属于strSet中的字符（地址）
char* strpbrk(const char* str, const char* strSet) {
    const char* elem;
    for (;*str != '\0'; ++str) {
        for (elem = strSet; *elem != '\0'; ++elem) {
            if (*str == *elem) {
                return (char*)str;
            }
        }
    }
    return NULL;
}
char* strtok(char* str, const char* delim) {
    static char* last;
    strtok_s(str, delim, &last);
}
//strtok内静态变量会导致线程不安全
char* strtok_s(char* str, const char* delim, char** context) {
    //指向即将到来的分隔符处。
    char* end;
    //*context记录上次分隔符后一个位置，即新子串开始处。

    //不是首次使用，记为上次结束的后一个位置，即新子串开始处。
    if (!str) {
        str = *context;
    }

    //可能开头都是分隔符，跳过开头的若干分隔符。
    str += strspn(str, delim);

    //已到字符串结束(*context==0)
    if (*str == '\0'){
        //*context = str;
        return NULL;
    }

    //寻找最长不含分隔符的子串
    end = str + strcspn(str, delim);
    if (*end == '\0'){
        *context = end;
        return str;
    }

    //将分割符处设为字符串结束符'\0'，返回分隔符前的子串位置。
    *end = '\0';
    *context = end + 1;
    return str;
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

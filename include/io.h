#ifndef __IO_H
#define __IO_H
#include"types.h"

//向端口port输出一字节数据data
static inline void __attribute__((always_inline)) outb(uint16_t port, uint8_t data) {
    asm volatile ("outb %b0,%1"
        ::"a"(data),"id"(port)
        );
}

//向端口port输出两字节数据data
static inline void __attribute__((always_inline)) outw(uint16_t port, uint16_t data) {
    asm volatile ("outw %w0,%1"
        ::"a"(data),"id"(port)
        );
}

//将%ds:addr内存处word_count个字写入端口port
static inline void __attribute__((always_inline)) outsw(uint16_t port, void* addr, uint32_t word_count) {
    asm volatile("cld;\n\t""rep outsw;\n\t"
        :"+S"(addr), "+c"(word_count)//"+"必须写在input处，表示又读又写
        :"d"(port));
}

//从端口port读取一字节数据
static inline uint8_t __attribute__((always_inline)) inb(uint16_t port) {
    uint8_t data;
    asm volatile ("inb %1,%b0"
        :"=a"(data)
        :"id"(port));
    return data;
}

//从端口port读取两字节数据
static inline uint16_t __attribute__((always_inline)) inw(uint16_t port) {
    uint16_t data;
    asm volatile ("inb %1,%w0"
        :"=a"(data)
        :"id"(port));
    return data;
}

//从端口port读word_count个字写入%es:addr内存处
static inline void __attribute__((always_inline)) insw(uint16_t port, void* addr, uint32_t word_count) {
    asm volatile ("cld;\n\t""rep insw"
        :"+D"(addr), "+c"(word_count):"d"(port)
        :"memory");
}
#endif
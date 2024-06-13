#ifndef __INT_H
#define __INT_H

#define cli() asm("cli")
#define sti() asm("sti")
#define nop() asm("nop")
#define EFLAGS_IF 0x00000200    //第9位为IF标志位，interrupt enable flag

#include "types.h"

#define EFLAGS() ({             \
uint32_t __eflags__;            \
__asm__ volatile("pushfl;\n\t"  \
    "popl %0"                   \
    :"=g"(__eflags__)           \
    );                          \
__eflags__;                     \
})

//返回操作前是否开中断
static inline uint32_t __attribute__((always_inline)) disable_int() {
    //32位会被截断，因此仍采用二值返回..直接返回32位罢了
    uint32_t isEnable = (EFLAGS() & EFLAGS_IF);
    asm volatile ("cli");
    return isEnable;
}
//返回操作前是否开中断
static inline uint32_t __attribute__((always_inline)) enable_int() {
    uint32_t isEnable = (EFLAGS() & EFLAGS_IF);
    asm volatile ("sti");
    return isEnable;
}
void idt_init();
#endif
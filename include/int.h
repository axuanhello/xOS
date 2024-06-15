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
struct intr_stack {
    //手动压栈
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    
    //手动压栈，pushal
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t eesp;
    uint32_t ebx;//漏了，出bug!
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t int_no;

    //CPU自动压栈，error_code为额外添加
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    //有特权级变化：
    uint32_t esp;
    uint32_t ss;

}__attribute__((packed));
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
static inline void __attribute__((always_inline)) resume_int(uint32_t origin_status) {
    if (origin_status) {
        asm volatile("sti"); 
    }
    else {
        asm volatile("cli");
    }
}
void idt_init();
#endif
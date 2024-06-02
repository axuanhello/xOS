#include"mmu.h"
#include"types.h"
struct gatedesc idt[256];
extern uint32_t int_start, int_len;
void idt_init() {
    for (int i = 0;i <= 32;++i) {
        //SETGATE(idt[i], 0, 8, &int_start + i * int_len, 0);//啊！BUG!int_start要取地址！不是标号了
        //啊！！！bug！忘记了指针加法会自动乘上上指针指向类型的所占字节作系数！
        SETGATE(idt[i], 0, 8, (uint32_t)&int_start + i * int_len, 0);
    }
    //uint64_t idt_ptr = (uint64_t)(&idt) << 16 + sizeof idt - 1;//BUG!!!指针竟默认当成有符号数！（应该是编译后就是立即数，默认int了）当直接扩展位表示时，遵循符号扩展而不是零扩展！必须先强转为无符号再扩展！
    uint64_t idt_ptr=((uint64_t)((uint32_t)(&idt))<<16)+sizeof idt - 1;
    asm volatile("lidt %0"::"m"(idt_ptr));

}
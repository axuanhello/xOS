#include"mmu.h"
#include"string.h"
#include "print.h"
struct segdesc gdt[256];
extern void idt_init();
void __attribute__((noreturn)) main(void) {
    clear_screen();
    set_cursor(0);
    print("Hello kernel!");
    idt_init();
    //测试除零中断
    //int b = 1 / 0;//成功触发中断
    //asm("int $2");//success!!
    asm("hlt");
    for (;;);
}
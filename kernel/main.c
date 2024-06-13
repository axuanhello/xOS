#include"mmu.h"
#include"string.h"
#include "print.h"
#include "assert.h"
#include "int.h"
#include "mm.h"
struct segdesc gdt[256];
extern short ards_nr;
void __attribute__((noreturn)) main(void) {
    clear_screen();
    set_cursor(0);
    put_str("Hello kernel!");
    //assert(0);
    //save_disable_int();
    
    idt_init();
    enable_int();
    disable_int();
    put_uint(v2p(&ards_nr));//success!!
    //测试除零中断
    //int b = 1 / 0;//成功触发中断
    //asm("int $2");//success!!
    asm("hlt");
    for (;;);
}
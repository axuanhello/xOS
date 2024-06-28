#include "mmu.h"
#include "string.h"
#include "print.h"
#include "assert.h"
#include "int.h"
#include "mm.h"
#include "config.h"
#include "thread.h"
#include "lock.h"
void gdt_init();
spinlock_t lock;
extern struct task_struct* kmain_thread;
void test(void* arg) {
    char* s = arg;
    thread_unblock(kmain_thread);
    while (1) {
        spin_lock(&lock);
        //put_str(s);
        spin_unlock(&lock);
        asm("nop;nop;nop;nop;nop");
    }
}
void __attribute__((noreturn)) main(void) {
    clear_screen();
    set_cursor(0);
    gdt_init();
    idt_init();
    
    mem_init();
    init_main_thread();
    spin_lock_init(&lock);
    char* thread = "test  ";
    thread_create(test, thread);
    thread_create(test, " ");
    thread_create(test, " ");
    thread_block(TASK_BLOCKED);
    while (1) {
        spin_lock(&lock);
        //put_str("main  ");
        spin_unlock(&lock);
        asm("nop;nop;nop");
    }

    //disable_int();
    //put_uint(v2p(&ards_nr));//success!!
    //测试除零中断
    //int b = 1 / 0;//成功触发中断
    //asm("int $2");//success!!
    asm("hlt");
    for (;;) asm("hlt");
}
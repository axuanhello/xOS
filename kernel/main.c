#include "mmu.h"
#include "string.h"
#include "print.h"
#include "assert.h"
#include "int.h"
#include "mm.h"
#include "config.h"
void gdt_init();

void __attribute__((noreturn)) main(void) {
    clear_screen();
    set_cursor(0);
    put_str("Hello kernel!\n");
    
    //assert(0);
    //save_disable_int();
    gdt_init();
    idt_init();
    
    mem_init();
    void* p[20];
    p[0] = get_a_mapped_kernel_page();
    p[1] = get_a_mapped_kernel_page();
    p[2] = get_a_mapped_kernel_page();
    p[3] = get_a_mapped_kernel_page();
    p[4] = get_a_mapped_kernel_page();
    disable_int();
    for (int i = 0;i < 5;++i) {
        put_str("p[");put_int(i);put_str("]:");
        put_uinth((uint32_t)p[i]);
        put_char('\n');
    }

    free_a_mapped_kernel_page(p[1]);

    //p[5] = get_a_mapped_kernel_page();
    //put_str("p[");put_int(5);put_str("]:");
    //put_uinth((uint32_t)p[5]);
    //put_char('\n');

    p[6] = sys_malloc(0, PAGE_SIZE * 6);
    put_str("p[");put_int(6);put_str("]:");
    put_uinth((uint32_t)p[6]);
    put_char('\n');

    p[7] = get_a_mapped_kernel_page();
    put_str("p[");put_int(7);put_str("]:");
    put_uinth((uint32_t)p[7]);
    put_char('\n');

    p[8] = get_a_mapped_kernel_page();
    put_str("p[");put_int(8);put_str("]:");
    put_uinth((uint32_t)p[8]);
    put_char('\n');

    p[9] = get_a_mapped_kernel_page();
    put_str("p[");put_int(9);put_str("]:");
    put_uinth((uint32_t)p[9]);
    put_char('\n');

    *(int*)p[0] = 0x100;
    char* hh = "  how is every  thing going?  ";
    char* token = strtok(hh, " ");
    while (token) {
        put_str(token);
        put_char('\n');
        token = strtok(NULL, " ");
    }
    char* bb = " haha now  good ";
    token = strtok(bb, " ");
    while (token) {
        put_str(token);
        put_char('\n');
        token = strtok(NULL, " ");
    }
    char* context = NULL;
    char* cc = " A B C ";
    token = strtok_s(cc, " ",&context);
    while (token) {
        put_str(token);
        put_char('\n');
        token = strtok_s(NULL, " ",&context);
    }
    //enable_int();
    disable_int();
    //put_uint(v2p(&ards_nr));//success!!
    //测试除零中断
    //int b = 1 / 0;//成功触发中断
    //asm("int $2");//success!!
    asm("hlt");
    for (;;);
}
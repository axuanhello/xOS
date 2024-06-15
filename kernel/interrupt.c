#include"mmu.h"
#include"types.h"
#include"io.h"
#include"print.h"
#include "int.h"
struct gatedesc idt[256];
extern void (*int_entry_addr[])(void);
extern void do_keyboard();
extern void sendeoi();
void do_timer() {
    //put_str("Timer interrupt. cpl:");
    //put_int(cpl);
    
    //put_str("\t");
}
void int_dispatch(struct intr_stack* pintr_stack) {
    //外部中断
    if (pintr_stack->int_no >= 32) {
        switch (pintr_stack->int_no) {
        case 32:
            put_str("Timer Interrupt. ");
            do_timer();
            sendeoi();
            return;
        case 33:
            do_keyboard();
            sendeoi();
            return;
        case 39:
            sendeoi();
            return;
        case 47:
            sendeoi();
            return;
        default:
            sendeoi();
            return;
        }
    }
    //内中断
    else {
        set_cursor(0);
        put_str("Interrupt!\t");
        put_uint(pintr_stack->int_no);
        put_str("Error code:");
        put_uint(pintr_stack->error_code);
        for (;;) {
            asm("hlt");
        }
    }
    return;
}
static void pic_init() {
    //主片
    //写入ICW1，初始化连接方式：级联，中断信号触发方式：边沿触发，需要ICW4。0001,LTIM,ADI,SNGL,IC4
    outb(0x20, 0x11);
    //写入ICW2，设置起始中断向量号。T7-T3,ID2-0
    outb(0x21, 0x20);
    //写入ICW3，设置级联接口为S2。S7-S0
    outb(0x21, 0x04);
    //写入ICW4，x86必须写入。置EOI=0，需手动响应中断。000,SFNM,BUF,M/S(BUF=1有效),AEOI,uPM(x86=1)
    outb(0x21, 0x01);

    //从片
    outb(0xa0, 0x11);
    outb(0xa1, 0x28);//中断开始0x28
    outb(0xa1, 0x02);//设置级联主片口为2号
    outb(0xa1, 0x01);//EOI

    /*
    * OCW1(0x21/0xa1),屏蔽中断，M7-M0
    * OCW2(0x20/0xa0),设置中断结束和优先级，R,SL,EOI,0,0,L2-L0
    * OCW3(0x20/0xa0),设置特殊屏蔽方式和查询方式
    */
    //屏蔽非时钟中断
    //outb(0x21, 0xfe);
    //outb(0xa1, 0xff);

}


void idt_init() {
    pic_init();
    for (int i = 0;i < sizeof(idt)/sizeof(idt[0]);++i) {
        //SETGATE(idt[i], 0, 8, &int_start + i * int_len, 0);//啊！BUG!int_start要取地址！不是标号了
        //啊！！！bug！忘记了指针加法会自动乘上上指针指向类型的所占字节作系数！
        //SETGATE(idt[i], 0, 8, (uint32_t)&int_start + i * int_len, 0);
        SETGATE(idt[i], 0, 8, int_entry_addr[i], 0);
    }
    //uint64_t idt_ptr = (uint64_t)(&idt) << 16 + sizeof idt - 1;//BUG!!!指针竟默认当成有符号数！（应该是编译后就是立即数，默认int了）当直接扩展位表示时，遵循符号扩展而不是零扩展！必须先强转为无符号再扩展！
    uint64_t idt_ptr=((uint64_t)((uint32_t)(&idt))<<16)+sizeof idt - 1;

    asm volatile("lidt %0"::"m"(idt_ptr));
    //允许中断
    asm volatile("sti");

}
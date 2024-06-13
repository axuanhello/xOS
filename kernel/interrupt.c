#include"mmu.h"
#include"types.h"
#include"io.h"
#include"print.h"
struct gatedesc idt[256];
extern uint32_t int_start, int_len;
void timer_interrupt();
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
    //屏蔽其它中断
    outb(0x21, 0xfe);
    outb(0xa1, 0xff);

}
void do_timer(uint32_t cpl) {
    put_str("Timer interrupt. cpl:");
    put_int(cpl);
    
    put_str("\t");
}
void idt_init() {
    pic_init();
    for (int i = 0;i < 32;++i) {
        //SETGATE(idt[i], 0, 8, &int_start + i * int_len, 0);//啊！BUG!int_start要取地址！不是标号了
        //啊！！！bug！忘记了指针加法会自动乘上上指针指向类型的所占字节作系数！
        SETGATE(idt[i], 0, 8, (uint32_t)&int_start + i * int_len, 0);
    }
    SETGATE(idt[32], 0, 8, &timer_interrupt, 0);
    //uint64_t idt_ptr = (uint64_t)(&idt) << 16 + sizeof idt - 1;//BUG!!!指针竟默认当成有符号数！（应该是编译后就是立即数，默认int了）当直接扩展位表示时，遵循符号扩展而不是零扩展！必须先强转为无符号再扩展！
    uint64_t idt_ptr=((uint64_t)((uint32_t)(&idt))<<16)+sizeof idt - 1;

    asm volatile("lidt %0"::"m"(idt_ptr));
    //允许中断
    asm volatile("sti");

}
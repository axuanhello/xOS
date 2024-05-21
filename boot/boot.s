    .global _start  
.text
_start:
    .code16
    #关中断，32位中断表尚未建立
    cli

    #打开A20地址线
    inb $0x92,%al
    orb $0x2,%al
    outb %al, $0x92

    #加载GDT
    lgdt gdtpos

    #置cr0,PE位为1，进入保护模式
    movl %cr0,%eax
    orl $0x1,%eax
    movl %eax,%cr0
    /*
    * Segment Selector:
    * 15-3  Index
    * 2     TI table Idicator 0=GDT,1=LDT
    * 1-0   RPL requested privilege level
    */
    ljmp $0x8, $start32 #通过jmp设置cs选择子，使用ljmp可清空流水线
    
    
start32:
    .code32
    movw $0x10,%ax
    movw %ax,%ds    #设置数据段选择子为2号gdt项
    #接下来需要将操作系统剩余部分代码从硬盘转到内存，并跳转执行
    hlt


.p2align 2  #内存对齐到2^2字节处

gdt:
    /*
    * 高地址到低地址依次为：
    * base 31_24    (8bit)
    * G             Granualrity
    * D/B           Default operation size
    * L             64-bit code seg
    * AVL           Availabel for use by system software
    * Seg Limit 19_16(4bit)
    * P             Segment present
    * DPL           Descriptor privilege level
    * S             Descriptor type(0=system;1=code or data)
    * Type          Segment type
    * Base 23_16    (8bit)
    * Base 15_0
    * Seg Limit 15_0
    */
    .quad 0x0
    .quad 0x00cf98000000ffff
    .quad 0x004092000000ffff

gdtpos:
    .word . -gdt-1  #gdt项数量
    .long gdt       #gdt位置


.org 510  #0x1fe
.byte 0x55,0xaa

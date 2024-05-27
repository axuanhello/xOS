#include "config.h"                 
#嘿，没被当成注释。note: gcc编译选项中已添加-Iinclude，不必写成"inluce/config.h"
    .global _start, ards_buf, ards_nr, gdt, gdtpos
    .text
_start:
    .code16
    xor %eax,%eax
    xor %ebx,%ebx
    xor %ecx,%ecx
    xor %edx,%edx
    xor %ebp,%ebp
    xor %esi,%esi
    mov %ax,%ds
    mov %ax,%es
    mov $STACK_SEG,%ax
    mov %ax,%ss
    mov $STACK_START,%sp     #临时设置栈为9F000开始
    mov %sp,%bp
    
read_remain:
    clc     #清除CF位
# INT $0x13读扇区使用调用参数设置如下:
# ah = 0x02  读磁盘扇区到内存       al = 需要读出的扇区数量;
# ch = 磁道(柱面)号的低8位；        cl = 开始扇区(位0~5),磁道号高2位(位6~7);
# dh = 磁头号；                     dl = 驱动器号（如果是硬盘则位7要置位）；
# es:bx 指向数据缓冲区;	如果出错则CF标志置位,ah中是出错码.
    #指定读取写入的内存开始地址es:bx
    movw $endsector1,%bx
    #指定读取的开始扇区
    movw $0x2,%cx
    #指定读取扇区数量al=64，指定读取操作ah=2。64*512B=32KB
    movw $0x240,%ax
    #指定读取第一块磁盘
    movw $0x80,%dx
    int $0x13
    #cf=1，读取失败。当提供qemu模拟的磁盘太小，会出错
    jc read_remain

#获取内存布局，采用int 0x10,eax=0xe820
get_memory:   
    xor %ebx,%ebx
    #应确保此时es=0
    movw $ards_buf,%di      #es:di用于指向存放ards结构体数据区域!Debug!忘记es会左移了，写入了错误的内存区域。
    mov $0x534d4150,%edx    #需指定edx为"SMAP"
loop_get_memory:
    mov $ARDS_SIZE,%cx         #指定ards结构体字节长度
    movl $0xe820,%eax   #指定子功能号
    int $0x15    
    jc get_memory       #cf为1出错，重试。应该尝试其它办法。在此简化则不尝试
    add $ARDS_SIZE,%di  #缓冲区di递增
    incw ards_nr 
    cmp $0,%ebx         #ebx为0则读取完内存数据
    jne loop_get_memory 

    mov $8,%si
    movw ards_nr,%cx
    #不能确定这段代码是否正确，因为对ards还不是特别了解，是否存在不连续的情况？
    #按，int 0x15,eax=0xe820返回未排序的列表，其中可能包含未使用的条目，并在极少数情况下可能返回重叠区域。
    movw $4,%bp 
    xorl %eax,%eax          #eax必须清零，否则会保留中断返回的SMAP签名
    xorl %ebx,%ebx
    #这里就不考虑Low memory设备了
    #因此就应该有历史遗留的1MB的memory hole，直接先加上去1MB。这样算出来物理内存正常。
    addl $0x100000,%eax
calculate_memory_size:
    
      
    addl ards_buf(%si),%eax   #累加ards中所有的length_low。
    adcl %ds:ards_buf(%bp,%si),%ebx  #进位累加ards中所有的length_high
    add $ARDS_SIZE,%si
    loop calculate_memory_size

    movl %eax,memory_size_low
    movl %ebx,memory_size_high
start_enter_32:    
    #关中断，32位中断表尚未建立
    cli

    #打开A20地址线，Fast A20 Gate法
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
    ljmp $0x8, $start32 #通过jmp设置cs选择子，使用jmp可清空流水线，刷新gdt缓存
    
    
start32:
    .code32
    movw $0x10,%ax
    movw %ax,%ds    #设置数据段选择子为2号gdt项
    movw %ax,%es
    movw %ax,%ss
    #临时设置栈顶，进入分页后原来的栈失效（因为没指向相同的物理地址），所以此期间就不要使用栈了，呃，不如不设sp。
    movl $0x7c00,%esp 
    #call bootmain 
    #TODO 接下来需要将操作系统剩余部分代码从硬盘转到内存，并跳转执行


#借Linus的代码一用:-)
setup_idt:
	lea ignore_int,%edx
	movl $0x00080000,%eax
	movw %dx,%ax		/* selector = 0x0008 = cs */
	movw $0x8E00,%dx	/* interrupt gate - dpl=0, present */

	lea idt,%edi
    #addl $112,%edi  #只设置缺页中断
	mov $256,%ecx
rp_sidt:
	movl %eax,(%edi)
	movl %edx,4(%edi)
	addl $8,%edi
	dec %ecx
	jne rp_sidt
	lidt idt_descr


    #开启分页机制
set_up_page:
    #TODO

    xorl %eax,%eax
    xorl %esi,%esi
    movl $1024,%ecx
clear_pde:
    movl %eax,PAGE_DIR_POS(%esi)
    addl $4,%esi
    loop clear_pde

create_pde:
    #设置第一个页目录项指向页表位置（PAGE_DIR_POS+PAGE_DIR_SIZE）
    movl $PAGE_TABLE_POS,%eax
    #addl $PAGE_DIR_SIZE,%eax
    #设置第一个页目录项属性(0号)
    orl $PAGE_P_W_U,%eax
    movl %eax,PAGE_DIR_POS

    #设置虚拟内存0xc0000000(3GB)所需要的页目录表指向页表同为0号页目录表项指向的页表，使3GB指向内核空间。(3G>>22=768号页表）
    #其所在物理内存位置为PAGE_DIR_POS+3G>>22*PAGE_DIR_SIZE(即3G>>20=0xc00)
    movl %eax,PAGE_DIR_POS+0xc00
    #hlt
    #分配3GB+4MB~4GB内核虚拟空间的页目录项所指向的页表，
    #指向从物理内存0x202000开始的空间，直至0x202000+((4GB-3GB-4MB)/4k*4=1020k)=0x301000用于存放页表
    movl $0x202000,%eax
    orl $PAGE_P_W_U,%eax
    #movl $0xc01,%esi   !!!!!大BUG!!!!
    movl $0xc04,%esi
    movl $255,%ecx
create_3G_4G_PDE:
    movl %eax,PAGE_DIR_POS(%esi)
    addl $PTE_SIZE,%esi
    #addl $PDE_SIZE,%eax !!!DEBUG!!!
    addl $PAGE_TABLE_SIZE,%eax
    loop create_3G_4G_PDE

    #创建0号页目录项对应的页表，虚拟地址与物理地址相等(0~1MB空间为恒等映射,1~4MB尚未映射)
    movl $PAGE_P_W_U,%eax
    xorl %esi,%esi
    movl $256,%ecx      #(只映射了前1MB)
create_pt_0:
    movl %eax,PAGE_TABLE_POS(,%esi,4)
    incl %esi
    #addl $PTE_SIZE,%eax !DEBUG!!!
    addl $PAGE_SIZE,%eax
    loop create_pt_0
    #sgdt gdtpos
    
    #接下来应修改原来寄存器指向的地址，使其指向3G~4G的虚拟内存
    #但是栈原来是比较随意地分配的（从实际物理内存16M开始往下）。这块物理内存还没有被映射到页表中！
    #虚拟地址本可以随意设值，只要对应页表映射的物理地址合法并存在即可。
    #不过要注意不要让压栈后进入的虚拟地址为内核代码的地址
    #可能还需要考虑和用户程序区分栈的问题。
    #TODO
    #算了，栈先小一点，毕竟只有0~1MB的物理地址被分配给页表了,500~7c00约30kB也勉强能用，后面有时间再分配
    #也可以给页表赋值，分配下内存。
    
    movl $KERNEL_VIRTUAL_MEM,%esp
    addl $0x7c00,%esp
    
  
    #cr3，页目录物理地址
    movl $PAGE_DIR_POS,%eax
    movl %eax,%cr3
    #设置cr0,PG位(31位)
    movl %cr0,%eax
    orl $0x80000000,%eax
    #开启分页前的测试
    #hlt
    movl %eax,%cr0
    #GDTR存的基址是线性地址，不是逻辑地址（虚拟地址）（如果是逻辑地址就递归分段映射了）
    #开启分页前即是物理地址。但若开启了分页就也不是物理地址了，也要经过分页的映射
    
    addl $KERNEL_VIRTUAL_MEM,gdtpos+2 
    lgdt gdtpos     #重新设置GDTR
    #hlt
    #接下来就是畅快的虚拟内存空间啦！得赶快（很久很久之后）实现获取空闲页
    movb $'V',0xc00b8000
    #hlt
    #测试代码
    call bootmain
    hlt


endcode:
.org 510  #0x1fe
.byte 0x55,0xaa
endsector1:
#存储ARDS结构体
ards_buf:
.fill 30,8,0    #注意编译器要求fill的,size不能大于8，也可使用.zero
ards_nr:
.word 0
memory_size:
memory_size_low:
.long 0
memory_size_high:
.long 0

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
    * DPL(2bit)     Descriptor privilege level
    * S             Descriptor type(0=system;1=code or data)
    * Type(4bit)    Segment type
    
    * Base 23_16    (8bit)
    
    * Base 15_0
    * Seg Limit 15_0
    */
NULL_GDT:
    .quad 0x0
CODE_DESC:
    #小端，高地址到低地址
    #Base=0,G=1,D/B=1,SegLim=0xfffff,P=1,DPL=0,S=1,Type=8(execute only)
    .quad 0x00cf98000000ffff
DATA_DESC:
    #Type=2(Read/Write)
    .quad 0x00cf92000000ffff
#STACK_DESC:#可以直接用data的描述符
#   .quad 0x004
GDT_LIMITS:
.fill 16,8,0        
GDT_EXTRA_LIMITS:
gdtpos:
    .word GDT_LIMITS -gdt-1  #低地址gdt表长度（字节）-1
    .long gdt       #高地址gdt基地址位置

.align 2
int_message:
.asciz "INTERRUPT!"
ignore_int:
    cld
    #movl $ignore_int-$int_message,%ecx
    xorl %edi,%edi
print_int_message:
    movb int_message(%edi),%al
    movb %al,0xb8000(,%edi,2)
    inc %edi
    cmp $0,%al
    jne print_int_message

    hlt
    #iret
.align 2
.word 0
idt_descr:
	.word 256*8-1		# idt contains 256 entries
	.long idt
idt:	.fill 256,8,0	
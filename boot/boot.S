    .global _start, ards_buf, ards_nr, endsector1, gdt, gdtpos
    .set STACK_SEG ,0x9000
    .set STACK_START,0xF000 
    .set ARDS_SIZE, 0x18    #按ACPI标准，建议使用24字节的ards结构体
    .set STACK_TOP_32,0xFFFFFF   #暂时假设打开32位后系统至少有这么多内存（16MB），可以在获取内存后选择合适的栈顶。
    .set PAGE_DIR_POS,0x200000   #将页目录放在0x200000(2MB)~0x200000+4k=0x204096
    
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
    #cf=1，读取失败。qemu模拟的磁盘太小，出错了
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
    ljmp $0x8, $start32 #通过jmp设置cs选择子，使用jmp可清空流水线，刷新gdt缓存
    
    
start32:
    .code32
    movw $0x10,%ax
    movw %ax,%ds    #设置数据段选择子为2号gdt项
    movw %ax,%es
    movw %ax,%ss
    #临时设置栈顶
    movl $STACK_TOP_32,%esp  
    #TODO 接下来需要将操作系统剩余部分代码从硬盘转到内存，并跳转执行
    
    #开启分页机制
set_up_page:
    #TODO
    
    #测试代码
    call bootmain
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
.fill 16,8,0        #注意可能超512B
GDT_EXTRA_LIMITS:
gdtpos:
    .word GDT_LIMITS -gdt-1  #低地址gdt表长度（字节）-1
    .long gdt       #高地址gdt基地址位置

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

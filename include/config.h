#ifndef __CONFIG_H
#define __CONFIG_H

#define STACK_SEG           0x9000
#define STACK_START         0xF000 
#define ARDS_SIZE           0x18        //按ACPI标准，建议使用24字节的ards结构体
#define STACK_TOP_32        0xFFFFFF    //暂时假设打开32位后系统至少有这么多内存（16MB），可以在获取内存后选择合适的栈顶。
#define PAGE_DIR_POS        0x200000    //将页目录放在0x200000(2MB)~0x200000+4k=0x201000
#define PAGE_DIR_SIZE       0x1000      //页目录表大小4k
#define PDE_SIZE            0x4         //页目录表项大小4B
#define PAGE_TABLE_POS      0x201000    //页表开始位置
#define PAGE_TABLE_SIZE     0x1000      //一张页表大小4k
//如果页表全部连续存放，则需要1k个页表，4MB空间，从0x201000~0x601000都是页表
#define PTE_SIZE            0x4         //一个页表项大小4B
#define PTE_NUM             1024        //一张页表含1024个页表项
#define PAGE_SIZE           0x1000      //页帧大小4k
#define PAGE_P              0x1
#define PAGE_W              0x2
#define PAGE_U              0x4
#define PAGE_P_W_U          0x7
#define KERNEL_VIRTUAL_MEM  0xc0000000  //虚拟内存3G~4G映射到内核物理空间。
#define KERNEL_START_VADDR  0xc0010000  //内核开始存放的虚拟地址
#define BOOT_END_SECTOR     10          //1~10扇区存bootloader等程序
#define KERNEL_SECTOR       11          //从第11个扇区开始存内核文件
#define KERNEL_PADDR    0x10000      //内核文件存放物理地址
#endif

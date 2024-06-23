#include"config.h"
#include"ards.h"
#include "types.h"
#include "bitmap.h"
#include "lock.h"
#include "string.h"
#include "mm.h"
//指向boot获取的ards信息位置，此后可复制到内核栈中。
struct ards* ards_buff = (struct ards*)(0x7c00 + ARDS_BUFF_OFFSET);
//由head.S从boot中复制
short ards_nr;
size_t global_memory_size;
struct mem_area {
    //每位描述每一页是否已被分配
    struct bitmap bitmap;
    size_t start_addr;
    size_t end_addr;
    spinlock_t lock;
};

struct mem_area kpmem;
struct mem_area kvmem;
struct mem_area upmem;
//struct mem_area uvmem;
//每个mem_desc描述同规格的内存仓库链。所有mem_desc构成一个不同规格的内存仓库描述
struct mem_desc {
    uint32_t block_size;//页帧中划分的每块内存块大小
    uint32_t block_num;//页帧中划分的总内存块数量
    struct mem_desc* next_desc;//下一个mem_desc
    struct list_node free_list;//仓库链头
};

//仅用于描述一页帧的分配情况。目前无用。没有Linux的mem_map
struct page {
    void* start_addr;       //允许用作内存的开始处，即obj开始处
    void* free_index_addr;  //描述空闲obj的索引的数组。如free[0]描述了某个空闲obj的索引，即start_addr+free[0]为真正空闲的obj位置
    uint32_t used_num;      //已经被使用的obj数量
    struct mem_desc* desc;  //所属的mem_desc
    struct list_node page_chain;//相同规格的仓库链
};
//描述页帧头元信息的物理组织结构；
struct page_head {
    struct mem_desc* desc;
    uint32_t used_num;
};
struct mem_desc mem_desc_chain[7];
extern void __attribute__((noreturn)) panic(const char* str);

static void init_ards() {
    //暂时放在0x100000(1MB)处，不和内核，页表冲突。
    struct ards* new_ards = (struct ards*)(KERNEL_VIRTUAL_MEM + 0x100000);
    global_memory_size = 0;
    for (int i = 0;i < ards_nr;++i) {
        new_ards[i] = ards_buff[i];
        if (ards_buff[i].type == 1) {
            global_memory_size += ards_buff[i].len;
        }
    }
    ards_buff = new_ards;
    //如果内存比32MB小，则不再继续。
    if (global_memory_size < 32 * 1024 * 1024) {
        panic("Too small memory!");
    }
    //对于更大的物理内存，我们最多只使用4GB物理内存。注意类型应为long long
    else if (global_memory_size > (4LL * 1024LL * 1024LL * 1024LL)) {
        global_memory_size = (4LL * 1024LL * 1024LL * 1024LL);
    }
}
static void init_mem_area() {
    /**
    * 物理内存低16MB保留给OS，不会被后续当作堆分配。boot阶段要确保已经映射了0MB-15MB
    * 其中15MB-16MB用于兼容可能的ISA，保留，不会被映射到虚拟内存。
    * 15MB向下用作栈
    * 2MB-4MB用作页目录表和部分页表
    * 4MB开始用作位图。1MB即可映射4GB内存，由于限制了可用物理内存最多不超过4GB，因此位图最多使用1MB+，故内核可以有10MB-的栈。当然，有能力可以实现动态扩展栈
    */
    kpmem.bitmap.map = (unsigned char*)(0x400000 + KERNEL_VIRTUAL_MEM);
    uint32_t reserved_mem = 16 * 1024 * 1024;
    size_t free_mem = global_memory_size - reserved_mem;
    size_t total_pages = free_mem / PAGE_SIZE;
    //内核的堆可用一半的物理内存
    kpmem.bitmap.byte_size = total_pages / 2 / 8;
    kpmem.start_addr = reserved_mem;
    kpmem.end_addr = reserved_mem + kpmem.bitmap.byte_size * 8 * PAGE_SIZE;
    spin_lock_init(&kpmem.lock);

    upmem.bitmap.map = kpmem.bitmap.map + kpmem.bitmap.byte_size;
    upmem.bitmap.byte_size = total_pages / 2 / 8;
    upmem.start_addr = kpmem.end_addr;
    upmem.end_addr = upmem.start_addr + upmem.bitmap.byte_size * 8 * PAGE_SIZE;
    spin_lock_init(&upmem.lock);

    memset(kpmem.bitmap.map, 0, kpmem.bitmap.byte_size);
    memset(upmem.bitmap.map, 0, upmem.bitmap.byte_size);

    kvmem.bitmap.map = upmem.bitmap.map + upmem.bitmap.byte_size;
    //内核固定可用堆虚拟内存大小1GB-16MB
    kvmem.bitmap.byte_size = 1 * 1024 * 1024 * 1024 / 8 / PAGE_SIZE - 16 * 1024 * 1024 / 8 / PAGE_SIZE;
    kvmem.start_addr = KERNEL_VIRTUAL_MEM + reserved_mem;
    kvmem.end_addr = 4 * 1024LL * 1024 * 1024;
    memset(kvmem.bitmap.map, 0, kvmem.bitmap.byte_size);
    spin_lock_init(&kvmem.lock);

}
void mem_init() {
    init_ards();
    init_mem_area();
}


//分配内存，返回内存地址，仅分配一页。若为虚拟内存，则尚未分配页表。若为物理内存，地址不应该超过4G。否则指针会溢出
static void* alloc_page(struct mem_area* mm) {

    spin_lock(&mm->lock);
    long long bit = get_zero_bit(&mm->bitmap);
    spin_unlock(&mm->lock);

    if (bit == -1) {
        return NULL;
    }
    //注意潜在的溢出风险
    return (void*)(uint32_t)(mm->start_addr + bit * PAGE_SIZE);
}
//分配连续的内存，未设置页表映射
static void* alloc_pages(struct mem_area* mm, uint32_t count) {

    spin_lock(&mm->lock);
    long long bit = get_zero_bits(&mm->bitmap, count);
    spin_unlock(&mm->lock);

    if (bit == -1) {
        return NULL;
    }
    else {
        //注意潜在的溢出风险
        return (void*)(uint32_t)(mm->start_addr + bit * PAGE_SIZE);
    }
}
//释放内存池对应的一页帧，尚未取消页表映射。
static void free_page(struct mem_area* mm, void* addr) {
    if ((uint32_t)addr & 0xfff) {
        panic("free_page:Not aligned address!");
    }
    long long bit = ((uint32_t)addr - mm->start_addr) >> PAGE_SHIFT;//误：又左移3位

    spin_lock(&mm->lock);
    clear_bitmap(mm->bitmap, bit);
    spin_unlock(&mm->lock);

}
//释放连续的内存帧，未取消页表映射
static void free_pages(struct mem_area* mm, void* addr, uint32_t count) {
    if ((uint32_t)addr & 0xfff) {
        panic("free_pages:Not aligned address!");
    }
    long long bit = ((uint32_t)addr - mm->start_addr) >> PAGE_SHIFT;

    spin_lock(&mm->lock);
    for (int i = 0;i < count;++i) {
        clear_bitmap(mm->bitmap, bit + i);
    }
    spin_unlock(&mm->lock);
}
//设置一个页表项映射，并不涉及内存池（除非需要使用一页表）
static void put_page(void* vaddr, void* paddr) {
    if ((uint32_t)paddr & 0xfff) {
        panic("put_page:Not aligned physical address!");
    }
    else if ((uint32_t)vaddr & 0xfff) {
        panic("put_page:Not aligned virtual address!");
    }
    uint32_t* pde_ptr = (uint32_t*)__pde_vaddr(vaddr);
    uint32_t* pte_ptr = (uint32_t*)__pte_vaddr(vaddr);
    //判断页表是否存在
    if (*pde_ptr & PAGE_P) {
        //页表存在
        //判断物理页帧是否存在
        if (*pte_ptr & PAGE_P) {
            panic("put_page:Already exist page frame!");
        }
        else {
            *pte_ptr = (uint32_t)paddr | PAGE_P_W_U;
        }
    }
    //页表不存在，需先创建页表。从内核物理堆中分配
    else {
        /**
        * 这一部分的物理内存没有被添加到虚拟内存中，
        * 理论上不会被直接破坏。
        * 回收时可以直接通过页表地址回收。
        * 如果是内核，一般不用回收页表。
        */
        uint32_t pt_paddr = (uint32_t)alloc_page(&kpmem);
        if (!pt_paddr) {
            panic("put_page:out of physical memory!");
        }
        *pde_ptr = pt_paddr | PAGE_P_W_U;
        //页表项全体清零。pte_ptr高20位即页表开始地址
        memset((void*)((uint32_t)pte_ptr & 0xfffff000), 0, PAGE_TABLE_SIZE);
        *pte_ptr = (uint32_t)paddr | PAGE_P_W_U;
    }
}
//取消一个页表项映射
static void unmap_page(void* vaddr, void* paddr) {
    if ((uint32_t)paddr & 0xfff) {
        panic("unmap_page:Not aligned physical address!");
    }
    else if ((uint32_t)vaddr & 0xfff) {
        panic("unmap_page:Not aligned virtual address!");
    }
    uint32_t* pde_ptr = (uint32_t*)__pde_vaddr(vaddr);
    uint32_t* pte_ptr = (uint32_t*)__pte_vaddr(vaddr);
    //判断页表是否存在
    if (*pde_ptr & PAGE_P) {
        //页表存在
        //判断物理页帧是否存在
        if (*pte_ptr & PAGE_P) {
            //页表项设置为0
            *pte_ptr = 0;
        }
        else {
            panic("unmap_page: Already unmaaped page frame!");
        }
    }
    else {
        panic("unmap_page: Already unmapped pde!");
    }
}
//获取不含头信息的一页，必须和free_a_mapped_kernel_page成对使用
void* get_a_mapped_kernel_page() {
    void* vaddr = alloc_page(&kvmem);
    if (vaddr) {
        void* paddr = alloc_page(&kpmem);
        put_page(vaddr, paddr);
        return vaddr;
    }
    return NULL;
}
//释放不含头信息的一页，必须和get_a_mapped_kernel_page成对使用
void free_a_mapped_kernel_page(void* vaddr) {
    if ((uint32_t)vaddr & 0xfff) {
        panic("free_a_mapped_kernel_page:Not aligned virtual address!");
    }
    free_page(&kvmem, vaddr);
    //可以直接用unmap_page()。但是要使用__v2p()，若有bug，有概率会解引用到不存在的页表项，触发中断。
    uint32_t* pde_ptr = (uint32_t*)__pde_vaddr(vaddr);
    uint32_t* pte_ptr = (uint32_t*)__pte_vaddr(vaddr);
    if (*pde_ptr & PAGE_P) {
        //页表存在
        //判断物理页帧是否存在
        if (*pte_ptr & PAGE_P) {
            //页表项设置为0
            void* paddr = (void*)(*(pte_ptr) & 0xfffff000);
            free_page(&kpmem, paddr);
            *pte_ptr = 0;
        }
        else {
            panic("free_a_mapped_kernel_page: Already unmaaped page frame!");
        }
    }
    else {
        panic("free_a_mapped_kernel_page: Already unmapped pde!");
    }

}
void* sys_malloc(enum mem_flags flag, size_t size) {
    //BUG! 加优先级高于移位
    uint32_t page_num = (size >> PAGE_SHIFT);
    if (size % PAGE_SIZE) {
        page_num += 1;
    }
    void* vaddr;
    struct mem_area* vmem;
    struct mem_area* pmem;
    if (flag == KERNEL_MEM_FLAG) {
        vmem = &kvmem;
        pmem = &kpmem;
    }
    else if (flag == USER_MEM_FLAG) {
        /**
        * TODO: 用户空间虚拟内存分配
        */
        //vmem = &uvmem;
        pmem = &upmem;
        panic("Not implemented user malloc.");
        return NULL;
    }
    if (page_num == 1) {
        vaddr = alloc_page(vmem);
    }
    else {
        vaddr = alloc_pages(vmem, page_num);
    }
    if (!vaddr) {
        return NULL;
    }

    void* nvaddr = vaddr;
    uint32_t need = page_num;
    void* paddr;
    while (need--) {
        paddr = alloc_page(pmem);
        if (paddr) {
            put_page(nvaddr, paddr);
        }
        else {

            /**
            * TODO: 应回收此前分配的物理内存。
            */
            return NULL;
        }
        nvaddr += PAGE_SIZE;
    }
    return vaddr;
}
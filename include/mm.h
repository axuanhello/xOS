#ifndef __MM_H
#define __MM_H

#define __pte_vaddr(__var_vaddr) (0xffc00000 | ((unsigned int)__var_vaddr >> 12 << 2))

#define __pde_vaddr(__var_vaddr) (0xfffff000 | ((unsigned int)__var_vaddr >> 22 << 2))

#define __v2p(__var_vaddr) ((*(unsigned int *)(__pte_vaddr(__var_vaddr)))&0xfffff000|(unsigned int)(__var_vaddr)&0xfff)
void mem_init();
void* get_a_mapped_kernel_page();
void free_a_mapped_kernel_page(void* vaddr);
enum mem_flags {
    KERNEL_MEM_FLAG,
    USER_MEM_FLAG
};
void* sys_malloc(enum mem_flags flag, size_t size);
#endif
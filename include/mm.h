#ifndef __MM_H
#define __MM_H

#define pte_vaddr(__var_vaddr) (0xffc00000 | ((unsigned int)__var_vaddr >> 12 << 2))

#define pde_vaddr(__var_vaddr) (0xfffff000 | ((unsigned int)__var_vaddr >> 22 << 2))

#define v2p(__var_vaddr) (*(unsigned int *)(pte_vaddr(__var_vaddr))&0xfffff000|(unsigned int)(__var_vaddr)&0xfff)
#endif
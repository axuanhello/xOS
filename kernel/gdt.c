#include "types.h"
#include "mmu.h"
struct segdesc gdt[256];
//最高16位无用。写成静态全局变量仅仅是为了不写扩展汇编:)
static uint64_t gdt_ptr = 0;
void gdt_init() {
    struct segdesc* boot_gdt_addr;
    uint16_t boot_gdt_limit;
    asm volatile("sgdt gdt_ptr");
    boot_gdt_addr = (struct segdesc*)(uint32_t)(gdt_ptr >> 16);
    boot_gdt_limit = gdt_ptr & 0xffff;
    for (int i = 0;i < (boot_gdt_limit + 1) / sizeof(gdt[0]);++i) {
        gdt[i] = boot_gdt_addr[i];
    }
    gdt_ptr = (uint64_t)((uint32_t)gdt) << 16|boot_gdt_limit;
    asm volatile("lgdt gdt_ptr");
}
#ifndef __ELF_H
#define __ELF_H

#define EI_NIDENT 16
#include "types.h"
//#define ELF_MAGIC 0x464C457FU   // 0x7F,'E','L','F'
typedef struct Elf32_Ehdr {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phunm;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
}elf_hdr;

typedef struct Elf32_Phdr {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
}elf_phdr;
#endif
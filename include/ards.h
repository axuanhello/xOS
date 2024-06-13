#ifndef __ARDS_H
#define __ARDS_H
#include "types.h"
struct ards {
    uint64_t base;
    uint64_t len;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

#endif
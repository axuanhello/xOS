#include"config.h"
    .global _start
_start:
#将ards数量存到本地变量中
    movl $ARDS_NR_OFFSET, %ebx
    movl 0x7c00(%ebx),%eax
    movl %eax,ards_nr
#可以直接使用jmp main，但是生成的是相对寻址
#带上*，不希望使用PC相对寻址（如果此时才开启分页则不能用相对寻址）
    movl $main,%eax
    jmp *%eax
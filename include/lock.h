#ifndef __LOCK_H
#define __LOCK_H
#include "list.h"
typedef struct spinlock {
    volatile int value;
}spinlock_t;

typedef struct semaphore {
    volatile int count;
    struct list_node wait_list;
    //没想到更好的办法，只能用自旋锁实现对信号量的互斥修改操作。
    spinlock_t lock;
}sem_t;

//仅在编译期生效。
#define barrier() asm volatile ("":::"memory")

/*
* 如果plock指向的值为old，那么会它被设为new，并返回原值(old)；
* 否则，plock保持原值，返回原值。
*/
static inline  uint32_t __attribute__((always_inline)) compare_and_swap(volatile uint32_t* plock, uint32_t old, uint32_t new) {
    uint32_t prev;
    asm volatile(
        //如果%eax与operand2相等，则operand2被赋值为operand1，
        //否则%eax被赋值为operand2
        "lock cmpxchgl %1,%2"
        :"=a"(prev)
        : "r"(new), "m"(*plock), "a"(old)
        : "memory"
        );
    return prev;
}

#define spin_lock_init(plock) (plock)->value=0

static inline void __attribute__((always_inline)) spin_lock(spinlock_t* lock) {
    while (!compare_and_swap(&(lock->value), 0, 1));
}
static inline void __attribute__((always_inline)) spin_unlock(spinlock_t* lock) {
    asm volatile (
        "movl $0,%0"
        :"=m"(lock->value)
        ::"memory"
        ); 
}

#endif
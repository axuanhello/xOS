#ifndef __THREAD_H
#define __THREAD_H
#include "types.h"
#include "list.h"
#define TASK_READY (1<<0)
#define TASK_RUNNING (1<<1)
#define TASK_WAITING (1<<2)
#define TASK_BLOCKED (1<<3)
#define TASK_KBD_BLOCKED (1<<4)
#define TASK_TERMINATED (1<<5)

struct thread_entry_stack {
    void (*eip)(void(*)(void*), void*);
    void* padding;
    void (*func)(void*);
    void* arg;
};

struct switch_context {
    uint32_t eip;
    uint32_t esp;//线程内核态下的栈顶

};
struct task_struct {
    //uint32_t* kernel_mode_stack;
    struct switch_context context;
    uint32_t pid;
    uint32_t wait_id;//设置等待的子线程id。若为0且状态为waiting，则等待任意子进程
    volatile uint32_t status;
    uint32_t priority;
    uint32_t ticks;
    uint64_t elapsed_ticks;
    struct task_struct* parent;
    struct list_node all;
    struct list_node ready;
    struct list_node blocked;
    struct list_node kbd_blocked;
    struct list_node zombie;
    uint32_t* pgdir;
};
struct task_list {
    struct list_node list;
    struct task_struct* task;  
};
struct task_struct* thread_create(void (*func)(void*), void* arg);
void init_main_thread();
void schedule();
void thread_block(uint32_t status);
void thread_unblock(struct task_struct* task);
//等待任意子线程完成
void thread_wait();
//等待特定子线程完成
void thread_wait4(uint32_t id);
#endif
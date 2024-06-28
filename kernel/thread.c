#include "thread.h"
#include "mm.h"
#include "config.h"
#include "string.h"
#include "cpu.h"
#include "list.h"
#include "int.h"
extern void __attribute__((noreturn)) panic(const char* str);
DECLARE_LIST(all_tasks);
DECLARE_LIST(ready_tasks);
DECLARE_LIST(blocked_tasks);
DECLARE_LIST(waiting_tasks);
DECLARE_LIST(kbd_blocked_tasks);
DECLARE_LIST(zombie_tasks);

#define DEFAULT_TICKS 10
#define MAX_TASKS 4096
struct task_struct* kmain_thread;
struct cpu cpu = { 0,0 };
static uint32_t next_pid = 0;
struct task_struct* task_arr[MAX_TASKS]; //4*4K=16KB
void thread_exit();
static void  thread_entry(void (*func)(void*), void* arg) {
    func(arg);
    thread_exit();//还需要回收资源。
}
static void switch_to(struct task_struct* __next) {
    struct task_struct* prev = cpu.curr;
    if (__next != cpu.curr) {
        cpu.curr = __next;
        __asm__ volatile(
            /*保存当前线程的esi,edi,ebx.
            用于保存上下文环境。在非时钟中断时引起的线程调度*/
            "pushl %%esi\n\t"
            "pushl %%edi\n\t"
            "pushl %%ebx\n\t"
            /*保存当前线程的中断标志*/
            "pushfl\n\t"
            "pushl %%ebp\n\t"
            /*保存当前线程的内核栈顶*/
            "movl %%esp,%0\n\t"
            /*保存当前线程的eip*/
            "movl $1f,%1\n\t"
            /*恢复之前线程（即将被调度上处理机）的内核栈顶*/
            "movl %2,%%esp\n\t"    /*BUG!写成了%1*/
            /*恢复eip */
            /*将cs:eip跳转到之前线程继续执行*/
            "jmp *%3\n\t"
            "1:\n\t"
            "popl %%ebp\n\t"
            "popfl\n\t"
            "popl %%ebx\n\t"
            "popl %%edi\n\t"
            "popl %%esi\n\t"
            :"=m"(prev->context.esp), "=m"(prev->context.eip)
            : "m"(__next->context.esp), "r"(__next->context.eip)        /*啊！又是bug!不能m*/
            );
    }
}
struct task_struct* thread_create(void (*func)(void*), void* arg) {
    struct task_struct* thread = get_a_mapped_kernel_page();
    memset(thread, 0, sizeof(struct task_struct));
    thread->status = TASK_READY;
    thread->ticks = DEFAULT_TICKS;
    thread->elapsed_ticks = 0;
    thread->pgdir = NULL;
    thread->parent = cpu.curr;
    //设置id
    if (task_arr[next_pid]) {
        int i = 1;
        for (;i < MAX_TASKS;++i) {
            if (task_arr[i] == 0) {
                task_arr[i] = thread;
                thread->pid = i;
                next_pid = (i + 1)%MAX_TASKS;
                break;
            }
        }
        if (i == MAX_TASKS) {
            panic("Too many tasks! Cannot allocate id.");
        }
    }
    else {
        task_arr[next_pid] = thread;
        thread->pid = next_pid;
        next_pid=(next_pid+1)%MAX_TASKS;
    }
    append_node(&all_tasks, &thread->all);
    append_node(&ready_tasks, &thread->ready);
    /*
        thread->context.esp = (uint32_t)thread + PAGE_SIZE - sizeof(struct intr_stack);
        asm volatile(
            "movl %0,%%esp\n\t"
            "pushl %1\n\t"
            "pushl %2\n\t"
            "call %3\n\t"
            ::""
            );*/
    thread->context.esp = (uint32_t)thread + PAGE_SIZE - sizeof(struct intr_stack) - sizeof(struct thread_entry_stack) + 4;
    thread->context.eip = (uint32_t)thread_entry;
    struct thread_entry_stack* s = (struct thread_entry_stack*)((uint32_t)thread->context.esp - 4);//同样的指针减法bug！
    s->func = func;
    s->arg = arg;
    s->eip = thread_entry;
    //switch_to(thread);
    
    /*
    cpu.curr = thread;
    asm volatile(
        "movl %0, %%esp;\n\t"//出BUG了！GP中断。
        //"movl %1,%%eax;\n\t"
        "jmp *%1;\n\t"  //bug在这！不应用m约束！
        ::"m"(thread->context.esp), "r"(s->eip)
        : "memory"
        );*/
    return thread;
}
void init_main_thread() {
    //kmain_thread应当在memory.c中已经分配位置。
    memset(kmain_thread, 0, sizeof(struct task_struct));
    kmain_thread->status = TASK_RUNNING;
    kmain_thread->ticks = DEFAULT_TICKS;
    kmain_thread->elapsed_ticks = 0;
    kmain_thread->pgdir = NULL;
    kmain_thread->pid = next_pid++;
    memset(task_arr, 0, sizeof(task_arr));
    task_arr[0] = kmain_thread;
    append_node(&all_tasks, &kmain_thread->all);
    cpu.curr = kmain_thread;
}
//实际上只有kbd_block和block
void thread_block(uint32_t set_status) {
    if ((set_status & (TASK_RUNNING|TASK_READY|TASK_WAITING))){
        panic("Wrong parameter:thread_block.");
    }
    cpu.curr->status = set_status;
    //调度切换线程。
    schedule();
}
//等待任意子线程完成
void thread_wait() {
    cpu.curr->wait_id = 0;
    cpu.curr->status = TASK_WAITING;
    schedule();
}
//等待特定子线程完成
void thread_wait4(uint32_t id) {
    cpu.curr->wait_id = id;
    cpu.curr->status = TASK_WAITING;
    schedule();
}
void thread_unblock(struct task_struct* task) {
    //可能被重复unblock
    if (task->status & (TASK_RUNNING | TASK_READY)) {
        return;
    }
    task->status = TASK_READY;
    //添加到就绪队列。等待被调度
    append_node(&ready_tasks, &task->ready);
}
void thread_exit() {
    cpu.curr->status = TASK_TERMINATED;
    schedule();
}
void schedule() {
    struct task_struct* curr = cpu.curr;
    struct task_struct* next;
    struct list_node* next_node;
    //时间片未到，但被事先设为了非运行态
    if (curr->ticks != 0 && !(curr->status & TASK_RUNNING|TASK_READY)) {
        switch (curr->status) {
        //可以将所有状态的任务用一数组作头
        case TASK_BLOCKED:
            append_node(&blocked_tasks, &curr->blocked);
            break;
        case TASK_WAITING:
            append_node(&waiting_tasks, &curr->blocked);
            break;
        case TASK_KBD_BLOCKED:
            append_node(&kbd_blocked_tasks, &curr->kbd_blocked);
            break;
        //理论上不宜直接设置为终止状态。
        case TASK_TERMINATED:
            //如果父进程为等待子进程状态，唤醒父进程
            if (curr->parent->status == TASK_WAITING&&curr->parent->wait_id==0) {
                curr->parent->status = TASK_READY;
                append_node(&ready_tasks, &curr->parent->ready);
            }
            else if (curr->parent->status == TASK_WAITING && curr->parent->wait_id == curr->pid) {
                curr->parent->status = TASK_READY;
                append_node(&ready_tasks, &curr->parent->ready);
            }
            /**
             * TODO:回收线程；
             * 
             */
            //append_node(&zombie_tasks, &curr->zombie);
            break;
        }
    }
    //时间片已到，原来还是运行态。
    else if (curr->ticks == 0 && curr->status == TASK_RUNNING) {
        curr->ticks = DEFAULT_TICKS;
        curr->status = TASK_READY;
        append_node(&ready_tasks, &curr->ready);
    }
    
    next_node = pop_first_node(&ready_tasks);
    if (next_node == NULL) {
        //执行idle进程。
    }
    next = node2data(struct task_struct, ready, next_node);
    next->status = TASK_RUNNING;
    switch_to(next);
}
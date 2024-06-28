#ifndef __CPU_H
#define __CPU_H
//extern struct task_struct;
struct cpu {
    struct task_struct* curr;
    unsigned long long run_ticks;
};
#endif 
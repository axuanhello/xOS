#ifndef __THREAD_H
#define __THREAD_H
#include "types.h"

enum task_status {
    TASK_NEW,
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_DIED
};

struct task_struct {
    uint32_t* kernel_mode_stack;
    uint32_t tid;
    enum task_status status;

};
#endif
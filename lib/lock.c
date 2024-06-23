#include "lock.h"

void sem_init(struct semaphore* sem, int val) {
    sem->count = val;
    init_list(&sem->wait_list);
    spin_lock_init(&sem->lock);
}
void sem_wait(struct semaphore* sem) {
    spin_lock(&sem->lock);
    sem->count--;
    
    if (sem->count < 0) {
        /*
        //由于线程一直阻塞，函数没有返回，因此栈上的数据一直是有效的
        struct task_list_node wait_node;
        wait_node.task=get_curr_thread();
        append_node(&wait_list,&wait_node.list_node);添加线程到信号量的等待队列中
        set_thread_stat(wait_node.task,BLOCK);设置线程状态为阻塞
        */
        //必须在进入睡眠队列、切换线程前释放锁，否则另一个线程也可能在等待锁
        spin_unlock(&sem->lock);//释放锁，允许其它信号量对coount和list的操作
        //schedule();//切换线程
        return;
    }
    spin_unlock(&sem->lock);

}
void sem_post(struct semaphore* sem) {
    spin_lock(&sem->lock);
    sem->count++;
    if (sem->count <= 0) {
        //弹出队头
        //struct task_list_node* node = node2data(struct task_list_node, struct list_node, pop_first_node(&sem->wait));
        
        //wakeup(node->task_struct);//唤醒线程
        spin_unlock(&sem->lock);
        return;
    }
    spin_unlock(&sem->lock);
}
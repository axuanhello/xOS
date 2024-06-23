#ifndef __LIST_H
#define __LIST_H
struct list_node {
    struct list_node* prior;
    struct list_node* next;
};
//获取结构体成员偏移量的常用办法：使用0指针指向成员后取地址
//也可以使用stddef.h中的offsetof宏
#define node2data(type,member,node_addr) (type*)((void*)(node_addr) - (void*)(&(((type*)0)->member)))
#define NODE_NOT_FOUND (-1)

void init_list(struct list_node* list);
void insert_node(struct list_node* new_node, struct list_node* position);
void insert_first_node(struct list_node* list, struct list_node* node);
void append_node(struct list_node* list, struct list_node* node);
struct list_node* pop_first_node(struct list_node* list);
struct list_node* pop_last_node(struct list_node* list);
char list_empty(struct list_node* list);
int find_node(struct list_node* list, struct list_node* node);
struct list_node*  remove_node(struct list_node* node);

#include "types.h"
size_t list_len(struct list_node* list);

#endif
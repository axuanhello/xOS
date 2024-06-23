#include "list.h"
#ifndef NULL
#define NULL ((void*)0)
#endif
void inline init_list(struct list_node* list) {
    list->next = list;
    list->prior = list;
}
void inline insert_node(struct list_node* new_node, struct list_node* position) {
    new_node->next = position;
    new_node->prior = position->prior;
    position->prior->next = new_node;
    position->prior = new_node;
}
void inline insert_first_node(struct list_node* list, struct list_node* node) {
    node->next = list->next;
    node->prior = list;
    list->next->prior = node;
    list->next = node;
}
void inline append_node(struct list_node* list, struct list_node* node) {
    node->prior = list->prior;
    node->next = list;
    list->prior->next = node;
    list->prior = node;
}
struct list_node* pop_first_node(struct list_node* list) {
    if (list->next == list) {
        return NULL;
    }
    struct list_node* first = list->next;
    list->next = first->next;
    first->next->prior = list;
    return first;
}
struct list_node* pop_last_node(struct list_node* list) {
    if (list->prior == list) {
        return NULL;
    }
    struct list_node* last=list->prior;
    last->prior->next = list;
    list->prior = last->prior;
    return last;
}
char inline list_empty(struct list_node* list) {
    return list->next == list;
}
//返回所在相对位置。不存在返回-1
int find_node(struct list_node* list, struct list_node* node) {
    int index = 0;
    struct list_node* cursor = list->next;
    while (cursor != list) {
        if (cursor == node) {
            return index;
        }
        cursor = cursor->next;
        ++index;
    }
    return NODE_NOT_FOUND;
}
inline struct list_node*  remove_node(struct list_node* node) {
    node->prior->next = node->next;
    node->next->prior = node->prior;
    return node;
}
size_t list_len(struct list_node* list) {
    size_t len = 0;
    struct list_node* cursor = list->next;
    while (cursor!=list) {
        ++len;
        cursor = cursor->next;
    }
    return len;
}
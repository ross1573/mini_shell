#include "list.h"


list* insert(void* __d, int __t, list* __b) {
    list* __l = malloc(sizeof(list));
    __l->data = __d;
    __l->tag = __t;
    __l->next = __b;
    return __l;
}

list* insert_end(void* __d, int __t, list* __b) {
    if (!__b) {
        return insert(__d, __t, __b);
    }
    else {
        list* __n = __b;
        for (; __n->next; __n = __n->next) {}
        list* __l = malloc(sizeof(list));
        __l->data = __d;
        __l->tag = __t;
        __l->next = NULL;
        __n->next = __l;
        return __b;
    }
}

int count(const list* __b) {
    if (!__b) return 0;
    
    int __c = 1;
    list* __l = __b->next;
    for (; __l; __l = __l->next) {
        __c += 1;
    }
    
    return __c;
}

list* find(int __k, list* __b) {
    if (is_empty(__b)) return NULL;
    list* __l = __b;
    for (; __l; __l = __l->next) {
        if (__l->tag == __k) {
            return __l;
        }
    }
    return NULL;
}

bool is_empty(const list* __b) {
    return !__b;
}

bool is_end(const list* __l) {
    if (!__l) assert("List is empty");
    if (!__l->next) return true;
    return false;
}

list* rm_node(list* __n) {
    list* __t = __n->next;
    free(__n);
    __n = NULL;
    return __t;
}

void rm_end(list* __b) {
    if (!__b) return;
    if (!__b->next) {
        rm_node(__b);
    }
    
    list* __n = __b;
    for (; __n->next->next; __n = __n->next) {}
    free(__n->next->data);
    __n->next = rm_node(__n->next);
}

void clear_list(list* __b) {
    for (; __b;) {
        free(__b->data);
        __b->data = NULL;
        __b = rm_node(__b);
    }
}

#ifndef __LIST_H__
#define __LIST_H__

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef __bool_true_false_are_defined
#define __bool_true_false_are_defined 1
typedef enum boolean {
    false,
    true
} bool;
#endif


/* Simple singly linked list */
// ================================ //
// Adding is only allowed at begin  //
// Removing is only allowed at end  //
// Access is only allowed by begin  //
// ================================ //

struct _list;
typedef struct _list list;

struct _list {
    list* next;
    int   tag;
    void* data;
};


/* crate node at begin */
list* insert(void* data, int tag, list* begin);             // data should manually allocated

/* create node at end */
list* insert_end(void* data, int tag, list* begin);

/* count number of nodes */
int count(const list* begin);

/* find node by key(int) */
list* find(int key, list* begin);

/* check if list is empty */
bool is_empty(const list* begin);

/* check if node is at end */
bool is_end(const list* node);

/* remove node */
list* rm_node(list* node);                                  // rm_node should be called only on clear functions

/* remove end */
void rm_end(list* begin);

/* remove all nodes */
void clear_list(list* begin);                               // clear_list frees data, multiple demension arrays aren't allowed

#endif /* __LIST_H__ */

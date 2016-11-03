#ifndef LIST_H
#define LIST_H

struct list_node {
        struct list_node *prev;
        struct list_node *next;
        void *data;
};

typedef struct list_node * list;

int l_insert_at(list *l, int index, struct list_node *n);
int l_push(list *l, struct list_node *n);

struct list_node *l_remove_at(list *l, int index);
struct list_node *l_remove(list *l, struct list_node *n);
struct list_node *l_pop(list *l);

int l_size(list *l);

#endif

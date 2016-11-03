#include "list.h"

int l_insert_at(list *l, int index, struct list_node *n)
{
        struct list_node *in;
        int i;

        if (!l) {
                return -1;
        } else if (!*l) {
                *l = n;
                n->prev = NULL;
                n->next = NULL;
                goto found;
        }

        for (in = *l, i = 0; in; in = in->next, ++i) {
                if (i == index) {
                        n->next = in;
                        
                        if (in->prev) {
                                (in->prev)->next = n;
                                n->prev = in->prev;
                        }

                        in->prev = n;
                        goto found;
                }
        }

        /* invalid index */
        return -2;

found:
        return 0;
}

int l_push(list *l, struct list_node *n)
{
        struct list_node *in;

        if (!l) {
                return -1;
        } else if (!*l) {
                *l = n;
                n->next = NULL;
                n->prev = NULL;
                return 0;
        }

        in = *l;
        while (in->next)
                in = in->next;

        in->next = n;
        n->next = NULL;
        n->prev = in;

        return 0;
}

struct list_node *l_remove_at(list *l, int index)
{
        struct list_node *in;
        int i;

        if (!l || !*l) {
                return NULL;
        } 

        for (in = *l, i = 0; in; in = in->next, ++i) {
                if (i == index) {
                        if (in->prev)
                                (in->prev)->next = in->next;
                        if (in->next)
                                (in->next)->prev = in->prev;

                        return in;
                }
        }

        return NULL;
}

struct list_node *l_remove(list *l, struct list_node *n)
{
        struct list_node *in;

        if (!l || !*l)
                return NULL;

        for (in = *l; in; in = in->next) {
                if (in == n) {
                        if (in->prev)
                                (in->prev)->next = in->next;
                        if (in->next)
                                (in->next)->prev = in->prev;

                        return in;
                }
        }

        return NULL;
}

struct list_node *l_pop(list *l)
{
        struct list_node *in;

        if (!l || !*l)
                return NULL;

        in = *l;
        if (in->next)
                (in->next)->prev = NULL;

        return in;
}

int l_size(list *l)
{
        struct list_node *in;
        int size;

        if (!l || !*l)
                return 0;

        size = 1;
        in = *l;
        while (in->next) {
                size++;
                in = in->next;
        }

        return size;
}

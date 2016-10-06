#include "user.h"

#include <string.h>

extern next_uid;

int add_user(char *name, char *realname)
{
        struct user *u = kmalloc(sizeof(struct user));
        u->next     = NULL;
        u->prev     = NULL;
        u->name     = kmalloc(strlen(name) + 1);
        u->realname = kmalloc(strlen(realname) + 1);
        strncpy(u->name, name, strlen(name) + 1);
        strncpy(u->realname, realname, strlen(realname) + 1);

        u->uid = (strncmp("root", name, 4) == 0) ? 0 : next_uid++;
        
        if (!user_list) {
                user_list = u;
        } else {
                struct user *n = user_list;
                while (n->next)
                        n = n->next;

                n->next = u;
                u->prev = n;
        }

        return u->uid;
}

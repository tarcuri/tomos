#include "user.h"

#include <string.h>

extern next_uid;

int add_user(char *name, char *realname)
{
        if (user_list) {
                struct user *c;
                for (c = user_list; c; c = c->next) {
                        if (strcmp(name, c->name) == 0)
                                return USER_EXISTS;
                }
        }

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



struct user *get_user(uint16_t uid)
{
        if (!user_list)
                return NULL;

        struct user *u;
        for (u = user_list; u; u = u->next) {
                if (u->uid == uid)
                        return u;
        }

        return NULL;
}

int get_uid(char *username, uint16_t *uid)
{
        if (!user_list)
                return USER_LIST_EMPTY;

        struct user *u;
        for (u = user_list; u; u = u->next) {
                if (strcmp(u->name, username) == 0) {
                        *uid = u->uid;
                        return 0;
                }
        }

        return USER_NOT_FOUND;
}

#ifndef USER_H
#define USER_H

#include <stdint.h>

#define MAX_USERNAME    16
#define MAX_REAL_NAME   64

struct user {
        struct user *next;
        struct user *prev;
        char *name;
        char *realname;
        uint16_t uid;
        uint16_t *pid_list;
};

struct user *user_list;

int add_user(char *name, char *realname);

#endif

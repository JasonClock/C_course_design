#ifndef C_ROLE_H
#define C_ROLE_H

#include "hotel.h"

typedef enum {
    USER_ADMIN = 1,
    USER_CUSTOMER = 2
} UserRole;

typedef struct UserOps {
    UserRole role;
    const char *name;
    int (*execute)(int choice, Room **roomList);
} UserOps;

const UserOps *role_get_ops(UserRole role);

#endif //C_ROLE_H


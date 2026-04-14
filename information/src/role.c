#include "role.h"

#include <stddef.h>

static int admin_execute(int choice, Room **roomList) {
    switch (choice) {
        case 1:
            hotel_list_all(*roomList);
            return 1;
        case 2:
            hotel_list_available(*roomList);
            return 1;
        case 3:
            hotel_reserve(*roomList);
            return 1;
        case 4:
            hotel_pay_reservation(*roomList);
            return 1;
        case 5:
            hotel_cancel_reservation(*roomList);
            return 1;
        case 6:
            hotel_check_in(*roomList);
            return 1;
        case 7:
            hotel_check_out(*roomList);
            return 1;
        case 8:
            hotel_query_by_guest(*roomList);
            return 1;
        case 9:
            hotel_print_statistics(*roomList);
            return 1;
        case 10:
            hotel_add_room(roomList);
            return 1;
        case 11:
            hotel_remove_room(roomList);
            return 1;
        case 0:
            return 0;
        default:
            return -1;
    }
}

static int customer_execute(int choice, Room **roomList) {
    switch (choice) {
        case 1:
            hotel_list_all(*roomList);
            return 1;
        case 2:
            hotel_list_available(*roomList);
            return 1;
        case 3:
            hotel_reserve(*roomList);
            return 1;
        case 4:
            hotel_pay_reservation(*roomList);
            return 1;
        case 5:
            hotel_check_in(*roomList);
            return 1;
        case 6:
            hotel_check_out(*roomList);
            return 1;
        case 7:
            hotel_query_by_guest(*roomList);
            return 1;
        case 0:
            return 0;
        default:
            return -1;
    }
}

static const UserOps ADMIN_OPS = {USER_ADMIN, "Administrator", admin_execute};
static const UserOps CUSTOMER_OPS = {USER_CUSTOMER, "Customer", customer_execute};

const UserOps *role_get_ops(UserRole role) {
    if (role == USER_ADMIN) {
        return &ADMIN_OPS;
    }
    if (role == USER_CUSTOMER) {
        return &CUSTOMER_OPS;
    }
    return NULL;
}


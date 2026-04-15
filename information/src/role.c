#include "role.h"

#include "hotel_storage.h"

#include <stddef.h>
#include <stdio.h>

/**
 * 功能：在房间结构发生变化后立即持久化数据。
 * @param roomList 当前房间链表头指针。
 */
static void save_after_room_structure_change(Room *roomList) {
    if (roomList == NULL) {
        return;
    }
    if (!hotel_storage_save(roomList, HOTEL_DATA_DIR)) {
        printf("Warning: failed to persist room changes to data_house.\n");
    }
}

/**
 * 功能：执行管理员菜单操作，并分发到对应业务函数。
 * @param choice 管理员输入的菜单上的编号。
 * @param roomList 房间链表头指针的地址，用于支持增删房间等修改头指针的操作。
 * @return 0 表示退出；1 表示执行成功；-1 表示无效选项或权限不允许。
 */
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
            save_after_room_structure_change(*roomList);
            return 1;
        case 11:
            hotel_remove_room(roomList);
            save_after_room_structure_change(*roomList);
            return 1;
        case 12:
            if (!hotel_storage_view(HOTEL_DATA_DIR)) {
                printf("Failed to open data_house data.\n");
            }
            return 1;
        case 13:
            if (!hotel_storage_reset(roomList, HOTEL_DATA_DIR)) {
                printf("Failed to reset database.\n");
            } else {
                printf("Database reset successfully.\n");
            }
            return 1;
        case 0:
            return 0;
        default:
            return -1;
    }
}

/**
 * 功能：执行普通用户菜单操作，并分发到对应业务函数。
 * @param choice 普通用户输入的菜单编号。
 * @param roomList 房间链表头指针的地址。
 * @return 0 表示退出；1 表示执行成功；-1 表示无效选项或权限不允许。
 */
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

/**
 * 功能：根据当前身份返回对应的操作表。
 * @param role 当前用户角色。
 * @return 返回角色对应的 UserOps 指针；若角色未知则返回 NULL。
 */
const UserOps *role_get_ops(UserRole role) {
    if (role == USER_ADMIN) {
        return &ADMIN_OPS;
    }
    if (role == USER_CUSTOMER) {
        return &CUSTOMER_OPS;
    }
    return NULL;
}


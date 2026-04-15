//
// Created by 18032 on 2026/4/14.
//

#include "menu.h"
#include "input.h"

#include <stdio.h>

/**
 * 功能：根据用户角色显示可执行菜单与命令提示。
 * @param role 当前用户角色，用于决定展示管理员或普通用户菜单。
 */
void menu_show(UserRole role) {
    printf("\n========== Hotel Room Reservation Management System ==========\n");
    if (role == USER_ADMIN) {
        /* 管理员模式支持通过 user 命令切换回普通用户。 */
        printf("Role: Administrator\n");
        printf("Command: type user to switch back to Customer\n");
        printf("1. View all rooms\n");
        printf("2. View available rooms\n");
        printf("3. Reserve a room\n");
        printf("4. Pay for reservation\n");
        printf("5. Cancel reservation\n");
        printf("6. Check in\n");
        printf("7. Check out\n");
        printf("8. Search by guest name\n");
        printf("9. Statistics\n");
        printf("10. Add room\n");
        printf("11. Delete room\n");
        printf("12. View data_house data\n");
        printf("13. Reset database\n");
    } else {
        /* 普通用户模式支持通过 sudo 命令切换到管理员。 */
        printf("Role: Customer (default)\n");
        printf("Command: type sudo to switch to Administrator\n");
        printf("1. View all rooms\n");
        printf("2. View available rooms\n");
        printf("3. Reserve a room\n");
        printf("4. Pay for reservation\n");
        printf("5. Check in\n");
        printf("6. Check out\n");
        printf("7. Search by guest name\n");
    }
    printf("0. Exit\n");
    printf("You can also type exit to quit.\n");
}

/**
 * 功能：读取角色选择输入（兼容旧流程，当前主流程默认普通用户启动）。
 * 说明：该函数无参数。
 * @return 读取成功返回角色编号，读取失败返回 -1。
 */
int menu_read_role(void) {
    int role;
    printf("\nSelect role:\n");
    printf("1. Administrator\n");
    printf("2. Customer\n");
    if (!input_read_int("Please choose your role: ", &role)) {
        return -1;
    }
    return role;
}

/**
 * 功能：读取菜单选项输入（兼容旧流程，当前主流程使用命令输入）。
 * 说明：该函数无参数。
 * @return 读取成功返回菜单编号，读取失败返回 -1。
 */
int menu_read_choice(void) {
    int choice;
    if (!input_read_int("Please choose an option: ", &choice)) {
        return -1;
    }
    return choice;
}

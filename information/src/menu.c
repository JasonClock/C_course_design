//
// Created by 18032 on 2026/4/14.
//

#include "menu.h"
#include "input.h"

#include <stdio.h>

void menu_show(UserRole role) {
    printf("\n========== Hotel Room Reservation Management System ==========\n");
    if (role == USER_ADMIN) {
        printf("Role: Administrator\n");
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
    } else {
        printf("Role: Customer\n");
        printf("1. View all rooms\n");
        printf("2. View available rooms\n");
        printf("3. Reserve a room\n");
        printf("4. Pay for reservation\n");
        printf("5. Check in\n");
        printf("6. Check out\n");
        printf("7. Search by guest name\n");
    }
    printf("0. Exit\n");
}

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

int menu_read_choice(void) {
    int choice;
    if (!input_read_int("Please choose an option: ", &choice)) {
        return -1;
    }
    return choice;
}

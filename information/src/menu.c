//
// Created by 18032 on 2026/4/14.
//

#include "menu.h"
#include "input.h"

#include <stdio.h>

void menu_show(void) {
    printf("\n========== Hotel Room Reservation Management System ==========\n");
    printf("1. View all rooms\n");
    printf("2. View available rooms\n");
    printf("3. Reserve a room\n");
    printf("4. Cancel reservation\n");
    printf("5. Check in\n");
    printf("6. Check out\n");
    printf("7. Search by guest name\n");
    printf("8. Statistics\n");
    printf("9. Add room\n");
    printf("10. Delete room\n");
    printf("0. Exit\n");
}

int menu_read_choice(void) {
    int choice;
    if (!input_read_int("Please choose an option: ", &choice)) {
        return -1;
    }
    return choice;
}

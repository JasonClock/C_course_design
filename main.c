#include <stdio.h>

#include "hotel.h"
#include "hotel_storage.h"
#include "menu.h"

static UserRole read_user_role(void) {
    int roleChoice;
    while (1) {
        roleChoice = menu_read_role();
        if (roleChoice == USER_ADMIN || roleChoice == USER_CUSTOMER) {
            return (UserRole)roleChoice;
        }
        printf("Please choose 1 (Administrator) or 2 (Customer).\n");
    }
}

int main(void) {
    UserRole role;
    const UserOps *ops;
    Room *roomList = NULL;

    if (!hotel_storage_load(&roomList, HOTEL_DATA_DIR)) {
        hotel_init(&roomList);
    }
    role = read_user_role();
    ops = role_get_ops(role);

    if (ops == NULL) {
        hotel_free(roomList);
        printf("Role initialization failed.\n");
        return 1;
    }

    printf("Login as %s.\n", ops->name);

    while (1) {
        int choice;
        int result;

        menu_show(role);
        choice = menu_read_choice();
        result = ops->execute(choice, &roomList);

        if (result == 0) {
            if (!hotel_storage_save(roomList, HOTEL_DATA_DIR)) {
                printf("Warning: failed to save data to data_house.\n");
            }
            hotel_free(roomList);
            printf("System exited.\n");
            return 0;
        }

        if (result < 0) {
            printf("Permission denied or invalid option for this role.\n");
        }
    }
}

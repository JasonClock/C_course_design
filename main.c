#include <stdio.h>

#include "hotel.h"
#include "menu.h"

int main(void) {

    Room *roomList = NULL;

    hotel_init(&roomList);

    while (1) {
        int choice;

        menu_show();
        choice = menu_read_choice();

        switch (choice) {
            case 1:
                hotel_list_all(roomList);
                break;
            case 2:
                hotel_list_available(roomList);
                break;
            case 3:
                hotel_reserve(roomList);
                break;
            case 4:
                hotel_cancel_reservation(roomList);
                break;
            case 5:
                hotel_check_in(roomList);
                break;
            case 6:
                hotel_check_out(roomList);
                break;
            case 7:
                hotel_query_by_guest(roomList);
                break;
            case 8:
                hotel_print_statistics(roomList);
                break;
            case 9:
                hotel_add_room(&roomList);
                break;
            case 10:
                hotel_remove_room(&roomList);
                break;
            case 0:
                hotel_free(roomList);
                printf("系统已退出。\n");
                return 0;
            default:
                printf("请输入有效数字。\n");
                break;
        }
    }
}

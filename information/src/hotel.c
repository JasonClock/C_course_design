//
// Created by 18032 on 2026/4/14.
//

#include "hotel.h"
#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *status_to_text(RoomStatus status) {
    if (status == ROOM_AVAILABLE) return "Available";
    if (status == ROOM_RESERVED) return "Reserved";
    return "Occupied";
}

static Room *create_room(int number, int type, double price) {
    Room *room = (Room *)malloc(sizeof(Room));
    if (room == NULL) {
        return NULL;
    }

    room->roomNumber = number;
    room->roomType = type;
    room->price = price;
    room->status = ROOM_AVAILABLE;
    room->guestName[0] = '\0';
    room->phone[0] = '\0';
    room->stayDays = 0;
    room->next = NULL;
    return room;
}

static void append_room(Room **head, Room *node) {
    Room *current;

    if (*head == NULL) {
        *head = node;
        return;
    }

    current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = node;
}

static Room *find_room(Room *head, int roomNumber) {
    while (head != NULL) {
        if (head->roomNumber == roomNumber) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

static void reset_guest_info(Room *room) {
    room->guestName[0] = '\0';
    room->phone[0] = '\0';
    room->stayDays = 0;
}

static void print_room(const Room *room) {
    printf("Room:%d | Type:%d | Price:%.2f | Status:%s", room->roomNumber, room->roomType, room->price, status_to_text(room->status));
    if (room->status != ROOM_AVAILABLE) {
        printf(" | Guest:%s | Phone:%s | Days:%d", room->guestName, room->phone, room->stayDays);
    }
    printf("\n");
}

static int input_room_number(void) {
    int roomNumber;
    if (!input_read_int("Enter room number: ", &roomNumber)) {
        return -1;
    }
    return roomNumber;
}

static int input_stay_days(void) {
    int days;
    if (!input_read_int("Enter number of stay days: ", &days) || days <= 0) {
        return -1;
    }
    return days;
}

static void input_guest_info(Room *room) {
    input_read_line("Enter guest name: ", room->guestName, NAME_LEN);
    input_read_line("Enter contact phone: ", room->phone, PHONE_LEN);
    room->stayDays = input_stay_days();
}

void hotel_init(Room **head) {
    int roomNumbers[] = {101, 102, 103, 201, 202, 203, 301, 302};
    int roomTypes[] = {1, 1, 2, 1, 2, 2, 3, 3};
    double roomPrices[] = {198.0, 198.0, 268.0, 218.0, 288.0, 288.0, 368.0, 368.0};
    int count = (int)(sizeof(roomNumbers) / sizeof(roomNumbers[0]));
    int i;

    for (i = 0; i < count; i++) {
        Room *room = create_room(roomNumbers[i], roomTypes[i], roomPrices[i]);
        if (room == NULL) {
            printf("Initialization failed: out of memory.\n");
            return;
        }
        append_room(head, room);
    }
}

void hotel_free(Room *head) {
    Room *next;
    while (head != NULL) {
        next = head->next;
        free(head);
        head = next;
    }
}

void hotel_list_all(Room *head) {
    if (head == NULL) {
        printf("No room information is available.\n");
        return;
    }
    while (head != NULL) {
        print_room(head);
        head = head->next;
    }
}

void hotel_list_available(Room *head) {
    int found = 0;
    while (head != NULL) {
        if (head->status == ROOM_AVAILABLE) {
            print_room(head);
            found = 1;
        }
        head = head->next;
    }
    if (!found) {
        printf("No available rooms at the moment.\n");
    }
}

void hotel_reserve(Room *head) {
    int roomNumber = input_room_number();
    Room *room;

    if (roomNumber < 0) {
        printf("Invalid room number input.\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("Room not found.\n");
        return;
    }

    if (room->status != ROOM_AVAILABLE) {
        printf("This room is not currently available.\n");
        return;
    }

    input_guest_info(room);
    if (room->stayDays <= 0 || room->guestName[0] == '\0') {
        reset_guest_info(room);
        printf("Reservation failed: incomplete input.\n");
        return;
    }

    room->status = ROOM_RESERVED;
    printf("Reservation successful: Room %d, estimated cost %.2f\n", room->roomNumber, room->price * room->stayDays);
}

void hotel_cancel_reservation(Room *head) {
    int roomNumber = input_room_number();
    Room *room;

    if (roomNumber < 0) {
        printf("Invalid room number input.\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("Room not found.\n");
        return;
    }

    if (room->status != ROOM_RESERVED) {
        printf("This room is not reserved, cannot cancel.\n");
        return;
    }

    room->status = ROOM_AVAILABLE;
    reset_guest_info(room);
    printf("Reservation cancelled.\n");
}

void hotel_check_in(Room *head) {
    int roomNumber = input_room_number();
    Room *room;

    if (roomNumber < 0) {
        printf("Invalid room number input.\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("Room not found.\n");
        return;
    }

    if (room->status == ROOM_OCCUPIED) {
        printf("This room is already occupied.\n");
        return;
    }

    if (room->status == ROOM_AVAILABLE) {
        printf("This room is not reserved. Proceeding with walk-in check-in.\n");
        input_guest_info(room);
        if (room->stayDays <= 0 || room->guestName[0] == '\0') {
            reset_guest_info(room);
            printf("Check-in failed: incomplete input.\n");
            return;
        }
    }

    room->status = ROOM_OCCUPIED;
    printf("Check-in successful: Room %d\n", room->roomNumber);
}

void hotel_check_out(Room *head) {
    int roomNumber = input_room_number();
    Room *room;
    double bill;

    if (roomNumber < 0) {
        printf("Invalid room number input.\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("Room not found.\n");
        return;
    }

    if (room->status != ROOM_OCCUPIED) {
        printf("This room is not currently occupied.\n");
        return;
    }

    bill = room->price * room->stayDays;
    printf("Check-out successful. Total bill: %.2f\n", bill);
    room->status = ROOM_AVAILABLE;
    reset_guest_info(room);
}

void hotel_query_by_guest(Room *head) {
    char name[NAME_LEN];
    int found = 0;

    if (!input_read_line("Enter guest name keyword: ", name, NAME_LEN) || name[0] == '\0') {
        printf("Input is empty.\n");
        return;
    }

    while (head != NULL) {
        if (head->status != ROOM_AVAILABLE && strstr(head->guestName, name) != NULL) {
            print_room(head);
            found = 1;
        }
        head = head->next;
    }

    if (!found) {
        printf("No matching guest record found.\n");
    }
}

void hotel_print_statistics(Room *head) {
    int available = 0;
    int reserved = 0;
    int occupied = 0;
    double expectedRevenue = 0.0;

    while (head != NULL) {
        if (head->status == ROOM_AVAILABLE) {
            available++;
        } else if (head->status == ROOM_RESERVED) {
            reserved++;
            expectedRevenue += head->price * head->stayDays;
        } else {
            occupied++;
            expectedRevenue += head->price * head->stayDays;
        }
        head = head->next;
    }

    printf("Available:%d Reserved:%d Occupied:%d\n", available, reserved, occupied);
    printf("Current estimated revenue: %.2f\n", expectedRevenue);
}

void hotel_add_room(Room **head) {
    int number;
    int type;
    double price;
    Room *newRoom;

    if (!input_read_int("Enter new room number: ", &number)) {
        printf("Invalid input.\n");
        return;
    }

    if (find_room(*head, number) != NULL) {
        printf("Room number already exists.\n");
        return;
    }

    if (!input_read_int("Enter room type (1/2/3): ", &type)) {
        printf("Invalid input.\n");
        return;
    }

    if (!input_read_double("Enter nightly price: ", &price) || price <= 0) {
        printf("Invalid price input.\n");
        return;
    }

    newRoom = create_room(number, type, price);
    if (newRoom == NULL) {
        printf("Add room failed: out of memory.\n");
        return;
    }

    append_room(head, newRoom);
    printf("Room added successfully.\n");
}

void hotel_remove_room(Room **head) {
    int number = input_room_number();
    Room *current = *head;
    Room *prev = NULL;

    if (number < 0) {
        printf("Invalid room number input.\n");
        return;
    }

    while (current != NULL && current->roomNumber != number) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Room not found.\n");
        return;
    }

    if (current->status == ROOM_OCCUPIED) {
        printf("This room is occupied and cannot be deleted.\n");
        return;
    }

    if (prev == NULL) {
        *head = current->next;
    } else {
        prev->next = current->next;
    }

    free(current);
    printf("Room deleted successfully.\n");
}

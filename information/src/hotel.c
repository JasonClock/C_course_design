//
// Created by 18032 on 2026/4/14.
//

#include "hotel.h"
#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Reservation *create_reservation(const char *guestName, const char *phone, int startDay, int endDay) {
    Reservation *node = (Reservation *)malloc(sizeof(Reservation));
    if (node == NULL) {
        return NULL;
    }

    node->startDay = startDay;
    node->endDay = endDay;
    node->checkedIn = 0;
    strncpy(node->guestName, guestName, NAME_LEN - 1);
    node->guestName[NAME_LEN - 1] = '\0';
    strncpy(node->phone, phone, PHONE_LEN - 1);
    node->phone[PHONE_LEN - 1] = '\0';
    node->next = NULL;
    return node;
}

static void free_reservations(Reservation *head) {
    Reservation *next;
    while (head != NULL) {
        next = head->next;
        free(head);
        head = next;
    }
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

static Room *create_room(int number, int type, double price) {
    Room *room = (Room *)malloc(sizeof(Room));
    if (room == NULL) {
        return NULL;
    }

    room->roomNumber = number;
    room->roomType = type;
    room->price = price;
    room->reservations = NULL;
    room->next = NULL;
    return room;
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

static int input_room_number(void) {
    int roomNumber;
    if (!input_read_int("Enter room number: ", &roomNumber)) {
        return -1;
    }
    return roomNumber;
}

static int input_period(int *startDay, int *endDay) {
    if (!input_read_int("Enter start day (integer): ", startDay)) {
        return 0;
    }
    if (!input_read_int("Enter end day (integer, exclusive): ", endDay)) {
        return 0;
    }
    if (*startDay < 0 || *endDay <= *startDay) {
        return 0;
    }
    return 1;
}

static int periods_overlap(int startA, int endA, int startB, int endB) {
    return !(endA <= startB || endB <= startA);
}

static int room_has_overlap(const Room *room, int startDay, int endDay) {
    const Reservation *current = room->reservations;
    while (current != NULL) {
        if (periods_overlap(startDay, endDay, current->startDay, current->endDay)) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

static void insert_reservation_sorted(Room *room, Reservation *node) {
    Reservation *current;
    Reservation *prev = NULL;

    current = room->reservations;
    while (current != NULL && current->startDay <= node->startDay) {
        prev = current;
        current = current->next;
    }

    if (prev == NULL) {
        node->next = room->reservations;
        room->reservations = node;
    } else {
        node->next = current;
        prev->next = node;
    }
}

static Reservation *find_exact_reservation(Room *room, int startDay, int endDay, Reservation **outPrev) {
    Reservation *current = room->reservations;
    Reservation *prev = NULL;

    while (current != NULL) {
        if (current->startDay == startDay && current->endDay == endDay) {
            if (outPrev != NULL) {
                *outPrev = prev;
            }
            return current;
        }
        prev = current;
        current = current->next;
    }

    if (outPrev != NULL) {
        *outPrev = NULL;
    }
    return NULL;
}

static Reservation *find_reservation_by_day(Room *room, int day) {
    Reservation *current = room->reservations;
    while (current != NULL) {
        if (current->startDay <= day && day < current->endDay) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static const char *reservation_state_text(int checkedIn) {
    return checkedIn ? "Checked-in" : "Reserved";
}

static void print_reservation(const Reservation *res, double price) {
    printf("  - [%d, %d) | %s | Guest:%s | Phone:%s | Bill:%.2f\n",
           res->startDay,
           res->endDay,
           reservation_state_text(res->checkedIn),
           res->guestName,
           res->phone,
           price * (res->endDay - res->startDay));
}

static void print_room(const Room *room) {
    const Reservation *res = room->reservations;

    printf("Room:%d | Type:%d | Price:%.2f\n", room->roomNumber, room->roomType, room->price);
    if (res == NULL) {
        printf("  - No reservations\n");
        return;
    }

    while (res != NULL) {
        print_reservation(res, room->price);
        res = res->next;
    }
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
        free_reservations(head->reservations);
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
    int startDay;
    int endDay;
    int found = 0;

    if (!input_period(&startDay, &endDay)) {
        printf("Invalid period input.\n");
        return;
    }

    while (head != NULL) {
        if (!room_has_overlap(head, startDay, endDay)) {
            printf("Room:%d | Type:%d | Price:%.2f\n", head->roomNumber, head->roomType, head->price);
            found = 1;
        }
        head = head->next;
    }

    if (!found) {
        printf("No available rooms for [%d, %d).\n", startDay, endDay);
    }
}

void hotel_reserve(Room *head) {
    int roomNumber;
    int startDay;
    int endDay;
    char guestName[NAME_LEN];
    char phone[PHONE_LEN];
    Room *room;
    Reservation *node;

    roomNumber = input_room_number();
    if (roomNumber < 0) {
        printf("Invalid room number input.\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("Room not found.\n");
        return;
    }

    if (!input_period(&startDay, &endDay)) {
        printf("Invalid period input.\n");
        return;
    }

    if (room_has_overlap(room, startDay, endDay)) {
        printf("Reservation failed: period conflict in this room.\n");
        return;
    }

    if (!input_read_line("Enter guest name: ", guestName, NAME_LEN) || guestName[0] == '\0') {
        printf("Reservation failed: guest name is required.\n");
        return;
    }

    if (!input_read_line("Enter contact phone: ", phone, PHONE_LEN) || phone[0] == '\0') {
        printf("Reservation failed: phone is required.\n");
        return;
    }

    node = create_reservation(guestName, phone, startDay, endDay);
    if (node == NULL) {
        printf("Reservation failed: out of memory.\n");
        return;
    }

    insert_reservation_sorted(room, node);
    printf("Reservation successful: Room %d, period [%d, %d).\n", room->roomNumber, startDay, endDay);
}

void hotel_cancel_reservation(Room *head) {
    int roomNumber;
    int startDay;
    int endDay;
    Room *room;
    Reservation *target;
    Reservation *prev;

    roomNumber = input_room_number();
    if (roomNumber < 0) {
        printf("Invalid room number input.\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("Room not found.\n");
        return;
    }

    if (!input_period(&startDay, &endDay)) {
        printf("Invalid period input.\n");
        return;
    }

    target = find_exact_reservation(room, startDay, endDay, &prev);
    if (target == NULL) {
        printf("No matching reservation for this period.\n");
        return;
    }

    if (target->checkedIn) {
        printf("Cannot cancel: this reservation is already checked in.\n");
        return;
    }

    if (prev == NULL) {
        room->reservations = target->next;
    } else {
        prev->next = target->next;
    }

    free(target);
    printf("Reservation cancelled.\n");
}

void hotel_check_in(Room *head) {
    int roomNumber;
    int day;
    Room *room;
    Reservation *res;

    roomNumber = input_room_number();
    if (roomNumber < 0) {
        printf("Invalid room number input.\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("Room not found.\n");
        return;
    }

    if (!input_read_int("Enter check-in day: ", &day) || day < 0) {
        printf("Invalid day input.\n");
        return;
    }

    res = find_reservation_by_day(room, day);
    if (res == NULL) {
        printf("No reservation covers this day.\n");
        return;
    }

    if (res->checkedIn) {
        printf("This reservation is already checked in.\n");
        return;
    }

    res->checkedIn = 1;
    printf("Check-in successful: Room %d, Guest %s.\n", room->roomNumber, res->guestName);
}

void hotel_check_out(Room *head) {
    int roomNumber;
    int day;
    Room *room;
    Reservation *res;
    Reservation *prev;
    double bill;

    roomNumber = input_room_number();
    if (roomNumber < 0) {
        printf("Invalid room number input.\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("Room not found.\n");
        return;
    }

    if (!input_read_int("Enter check-out day: ", &day) || day < 0) {
        printf("Invalid day input.\n");
        return;
    }

    res = find_reservation_by_day(room, day);
    if (res == NULL || !res->checkedIn) {
        printf("No checked-in reservation covers this day.\n");
        return;
    }

    bill = room->price * (res->endDay - res->startDay);

    prev = NULL;
    find_exact_reservation(room, res->startDay, res->endDay, &prev);
    if (prev == NULL) {
        room->reservations = res->next;
    } else {
        prev->next = res->next;
    }

    printf("Check-out successful. Total bill: %.2f\n", bill);
    free(res);
}

void hotel_query_by_guest(Room *head) {
    char name[NAME_LEN];
    int found = 0;

    if (!input_read_line("Enter guest name keyword: ", name, NAME_LEN) || name[0] == '\0') {
        printf("Input is empty.\n");
        return;
    }

    while (head != NULL) {
        Reservation *res = head->reservations;
        while (res != NULL) {
            if (strstr(res->guestName, name) != NULL) {
                printf("Room:%d | Type:%d | Price:%.2f\n", head->roomNumber, head->roomType, head->price);
                print_reservation(res, head->price);
                found = 1;
            }
            res = res->next;
        }
        head = head->next;
    }

    if (!found) {
        printf("No matching guest record found.\n");
    }
}

void hotel_print_statistics(Room *head) {
    int roomCount = 0;
    int reservationCount = 0;
    int checkedInCount = 0;
    double expectedRevenue = 0.0;

    while (head != NULL) {
        Reservation *res = head->reservations;
        roomCount++;

        while (res != NULL) {
            reservationCount++;
            if (res->checkedIn) {
                checkedInCount++;
            }
            expectedRevenue += head->price * (res->endDay - res->startDay);
            res = res->next;
        }

        head = head->next;
    }

    printf("Rooms:%d Reservations:%d Checked-in:%d\n", roomCount, reservationCount, checkedInCount);
    printf("Estimated revenue from all segments: %.2f\n", expectedRevenue);
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

    if (current->reservations != NULL) {
        printf("This room has reservations and cannot be deleted.\n");
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

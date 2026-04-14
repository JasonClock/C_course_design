//
// Created by 18032 on 2026/4/14.
//

#include "hotel.h"
#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PAYMENT_LOCK_TIMEOUT_SECONDS 300

static int is_leap_year(int year) {
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

static int days_in_month(int year, int month) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2) {
        return is_leap_year(year) ? 29 : 28;
    }
    return days[month - 1];
}

static int date_is_valid(Date date) {
    if (date.year < 1970 || date.month < 1 || date.month > 12) {
        return 0;
    }
    if (date.day < 1 || date.day > days_in_month(date.year, date.month)) {
        return 0;
    }
    return 1;
}

static int date_compare(Date a, Date b) {
    if (a.year != b.year) return a.year < b.year ? -1 : 1;
    if (a.month != b.month) return a.month < b.month ? -1 : 1;
    if (a.day != b.day) return a.day < b.day ? -1 : 1;
    return 0;
}

static int date_to_time_t(Date date, time_t *outTime) {
    struct tm tmValue;
    tmValue.tm_year = date.year - 1900;
    tmValue.tm_mon = date.month - 1;
    tmValue.tm_mday = date.day;
    tmValue.tm_hour = 0;
    tmValue.tm_min = 0;
    tmValue.tm_sec = 0;
    tmValue.tm_isdst = -1;

    *outTime = mktime(&tmValue);
    return *outTime != (time_t)-1;
}

static int days_between(Date startDate, Date endDate) {
    time_t startTs;
    time_t endTs;
    double seconds;

    if (!date_to_time_t(startDate, &startTs) || !date_to_time_t(endDate, &endTs)) {
        return 0;
    }

    seconds = difftime(endTs, startTs);
    if (seconds <= 0) {
        return 0;
    }
    return (int)(seconds / 86400.0);
}

static void print_date(Date date) {
    printf("%04d-%02d-%02d", date.year, date.month, date.day);
}

static int input_date(const char *label, Date *outDate) {
    Date d;
    char prompt[80];

    snprintf(prompt, sizeof(prompt), "Enter %s year: ", label);
    if (!input_read_int(prompt, &d.year)) return 0;

    snprintf(prompt, sizeof(prompt), "Enter %s month: ", label);
    if (!input_read_int(prompt, &d.month)) return 0;

    snprintf(prompt, sizeof(prompt), "Enter %s day: ", label);
    if (!input_read_int(prompt, &d.day)) return 0;

    if (!date_is_valid(d)) {
        return 0;
    }

    *outDate = d;
    return 1;
}

static int input_period(Date *startDate, Date *endDate) {
    if (!input_date("start", startDate)) {
        return 0;
    }
    if (!input_date("end (check-out)", endDate)) {
        return 0;
    }
    if (date_compare(*startDate, *endDate) >= 0) {
        return 0;
    }
    return 1;
}

static int periods_overlap(Date startA, Date endA, Date startB, Date endB) {
    return date_compare(endA, startB) > 0 && date_compare(endB, startA) > 0;
}

static Reservation *create_reservation(const char *guestName, const char *phone, Date startDate, Date endDate) {
    Reservation *node = (Reservation *)malloc(sizeof(Reservation));
    if (node == NULL) {
        return NULL;
    }

    node->startDate = startDate;
    node->endDate = endDate;
    node->checkedIn = 0;
    node->paymentStatus = PAYMENT_LOCKED;
    node->lockTime = time(NULL);
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

static int reservation_lock_expired(const Reservation *res, time_t now) {
    if (res->paymentStatus != PAYMENT_LOCKED) {
        return 0;
    }
    return difftime(now, res->lockTime) >= PAYMENT_LOCK_TIMEOUT_SECONDS;
}

static void cleanup_expired_locks_in_room(Room *room, time_t now) {
    Reservation *current = room->reservations;
    Reservation *prev = NULL;

    while (current != NULL) {
        Reservation *next = current->next;
        if (reservation_lock_expired(current, now)) {
            if (prev == NULL) {
                room->reservations = next;
            } else {
                prev->next = next;
            }
            free(current);
        } else {
            prev = current;
        }
        current = next;
    }
}

static void cleanup_expired_locks(Room *head) {
    time_t now = time(NULL);
    while (head != NULL) {
        cleanup_expired_locks_in_room(head, now);
        head = head->next;
    }
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

static int room_has_overlap(const Room *room, Date startDate, Date endDate) {
    const Reservation *current = room->reservations;
    while (current != NULL) {
        if (periods_overlap(startDate, endDate, current->startDate, current->endDate)) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

static void insert_reservation_sorted(Room *room, Reservation *node) {
    Reservation *current = room->reservations;
    Reservation *prev = NULL;

    while (current != NULL && date_compare(current->startDate, node->startDate) <= 0) {
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

static Reservation *find_exact_reservation(Room *room, Date startDate, Date endDate, Reservation **outPrev) {
    Reservation *current = room->reservations;
    Reservation *prev = NULL;

    while (current != NULL) {
        if (date_compare(current->startDate, startDate) == 0 && date_compare(current->endDate, endDate) == 0) {
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

static Reservation *find_reservation_by_date(Room *room, Date date) {
    Reservation *current = room->reservations;
    while (current != NULL) {
        if (date_compare(current->startDate, date) <= 0 && date_compare(date, current->endDate) < 0) {
            return current;
        }
        current = current->next;
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

static const char *payment_state_text(const Reservation *res) {
    return res->paymentStatus == PAYMENT_PAID ? "Paid" : "Locked-Unpaid";
}

static const char *reservation_state_text(const Reservation *res) {
    return res->checkedIn ? "Checked-in" : "Reserved";
}

static void print_reservation(const Reservation *res, double price) {
    int days = days_between(res->startDate, res->endDate);

    printf("  - [");
    print_date(res->startDate);
    printf(" -> ");
    print_date(res->endDate);
    printf(") | %s | %s | Guest:%s | Phone:%s | Bill:%.2f\n",
           reservation_state_text(res),
           payment_state_text(res),
           res->guestName,
           res->phone,
           price * days);
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
    cleanup_expired_locks(head);
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
    Date startDate;
    Date endDate;
    int found = 0;

    cleanup_expired_locks(head);

    if (!input_period(&startDate, &endDate)) {
        printf("Invalid period input.\n");
        return;
    }

    while (head != NULL) {
        if (!room_has_overlap(head, startDate, endDate)) {
            printf("Room:%d | Type:%d | Price:%.2f\n", head->roomNumber, head->roomType, head->price);
            found = 1;
        }
        head = head->next;
    }

    if (!found) {
        printf("No available rooms for [");
        print_date(startDate);
        printf(" -> ");
        print_date(endDate);
        printf(").\n");
    }
}

void hotel_reserve(Room *head) {
    int roomNumber;
    Date startDate;
    Date endDate;
    char guestName[NAME_LEN];
    char phone[PHONE_LEN];
    Room *room;
    Reservation *node;
    char payNow[8];

    cleanup_expired_locks(head);

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

    if (!input_period(&startDate, &endDate)) {
        printf("Invalid period input.\n");
        return;
    }

    if (room_has_overlap(room, startDate, endDate)) {
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

    node = create_reservation(guestName, phone, startDate, endDate);
    if (node == NULL) {
        printf("Reservation failed: out of memory.\n");
        return;
    }

    insert_reservation_sorted(room, node);
    printf("Reservation created and payment locked: Room %d, period [", room->roomNumber);
    print_date(startDate);
    printf(" -> ");
    print_date(endDate);
    printf(").\n");

    if (!input_read_line("Pay now? (y/n): ", payNow, (int)sizeof(payNow))) {
        printf("Payment not completed. Lock will expire in 5 minutes.\n");
        return;
    }

    if (payNow[0] == 'y' || payNow[0] == 'Y') {
        node->paymentStatus = PAYMENT_PAID;
        node->lockTime = 0;
        printf("Payment successful. Reservation confirmed.\n");
    } else {
        printf("Payment pending. Lock will expire in 5 minutes.\n");
    }
}

void hotel_pay_reservation(Room *head) {
    int roomNumber;
    Date startDate;
    Date endDate;
    Room *room;
    Reservation *res;

    cleanup_expired_locks(head);

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

    if (!input_period(&startDate, &endDate)) {
        printf("Invalid period input.\n");
        return;
    }

    res = find_exact_reservation(room, startDate, endDate, NULL);
    if (res == NULL) {
        printf("No matching reservation for this period.\n");
        return;
    }

    if (res->paymentStatus == PAYMENT_PAID) {
        printf("This reservation is already paid.\n");
        return;
    }

    res->paymentStatus = PAYMENT_PAID;
    res->lockTime = 0;
    printf("Payment successful. Reservation confirmed.\n");
}

void hotel_cancel_reservation(Room *head) {
    int roomNumber;
    Date startDate;
    Date endDate;
    Room *room;
    Reservation *target;
    Reservation *prev;

    cleanup_expired_locks(head);

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

    if (!input_period(&startDate, &endDate)) {
        printf("Invalid period input.\n");
        return;
    }

    target = find_exact_reservation(room, startDate, endDate, &prev);
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
    Date checkInDate;
    Room *room;
    Reservation *res;

    cleanup_expired_locks(head);

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

    if (!input_date("check-in", &checkInDate)) {
        printf("Invalid date input.\n");
        return;
    }

    res = find_reservation_by_date(room, checkInDate);
    if (res == NULL) {
        printf("No reservation covers this date.\n");
        return;
    }

    if (res->checkedIn) {
        printf("This reservation is already checked in.\n");
        return;
    }

    if (res->paymentStatus != PAYMENT_PAID) {
        printf("Check-in denied: payment is not completed.\n");
        return;
    }

    res->checkedIn = 1;
    printf("Check-in successful: Room %d, Guest %s.\n", room->roomNumber, res->guestName);
}

void hotel_check_out(Room *head) {
    int roomNumber;
    Date checkOutDate;
    Room *room;
    Reservation *res;
    Reservation *prev;
    double bill;

    cleanup_expired_locks(head);

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

    if (!input_date("check-out", &checkOutDate)) {
        printf("Invalid date input.\n");
        return;
    }

    res = find_reservation_by_date(room, checkOutDate);
    if (res == NULL || !res->checkedIn) {
        printf("No checked-in reservation covers this date.\n");
        return;
    }

    bill = room->price * days_between(res->startDate, res->endDate);

    prev = NULL;
    find_exact_reservation(room, res->startDate, res->endDate, &prev);
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

    cleanup_expired_locks(head);

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
    int paidCount = 0;
    int lockedCount = 0;
    double expectedRevenue = 0.0;

    cleanup_expired_locks(head);

    while (head != NULL) {
        Reservation *res = head->reservations;
        roomCount++;

        while (res != NULL) {
            reservationCount++;
            if (res->checkedIn) {
                checkedInCount++;
            }
            if (res->paymentStatus == PAYMENT_PAID) {
                paidCount++;
            } else {
                lockedCount++;
            }
            expectedRevenue += head->price * days_between(res->startDate, res->endDate);
            res = res->next;
        }

        head = head->next;
    }

    printf("Rooms:%d Reservations:%d Checked-in:%d Paid:%d Locked:%d\n",
           roomCount,
           reservationCount,
           checkedInCount,
           paidCount,
           lockedCount);
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

    cleanup_expired_locks(*head);

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

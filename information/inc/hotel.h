//
// Created by 18032 on 2026/4/14.
//

#ifndef C_HOTEL_H
#define C_HOTEL_H

#define NAME_LEN 50
#define PHONE_LEN 20

typedef enum {
    ROOM_AVAILABLE = 0,
    ROOM_RESERVED = 1,
    ROOM_OCCUPIED = 2
} RoomStatus;

typedef struct Reservation {
    int startDay;
    int endDay;
    int checkedIn;
    char guestName[NAME_LEN];
    char phone[PHONE_LEN];
    struct Reservation *next;
} Reservation;

typedef struct Room {
    int roomNumber;
    int roomType;
    double price;
    Reservation *reservations;
    struct Room *next;
} Room;
/*创建与删除房间*/
void hotel_init(Room **head);
void hotel_free(Room *head);
/*查询房间*/
void hotel_list_all(Room *head);
void hotel_list_available(Room *head);
/*对房间操作*/
void hotel_reserve(Room *head);
void hotel_cancel_reservation(Room *head);
void hotel_check_in(Room *head);
void hotel_check_out(Room *head);

void hotel_query_by_guest(Room *head);
void hotel_print_statistics(Room *head);

void hotel_add_room(Room **head);
void hotel_remove_room(Room **head);

#endif //C_HOTEL_H

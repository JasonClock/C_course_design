//
// Created by 18032 on 2026/4/14.
//

#include "hotel.h"
#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *status_to_text(RoomStatus status) {
    if (status == ROOM_AVAILABLE) return "空闲";
    if (status == ROOM_RESERVED) return "已预定";
    return "已入住";
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
    printf("房号:%d | 房型:%d | 价格:%.2f | 状态:%s", room->roomNumber, room->roomType, room->price, status_to_text(room->status));
    if (room->status != ROOM_AVAILABLE) {
        printf(" | 客人:%s | 电话:%s | 天数:%d", room->guestName, room->phone, room->stayDays);
    }
    printf("\n");
}

static int input_room_number(void) {
    int roomNumber;
    if (!input_read_int("请输入房号: ", &roomNumber)) {
        return -1;
    }
    return roomNumber;
}

static int input_stay_days(void) {
    int days;
    if (!input_read_int("请输入入住天数: ", &days) || days <= 0) {
        return -1;
    }
    return days;
}

static void input_guest_info(Room *room) {
    input_read_line("请输入客人姓名: ", room->guestName, NAME_LEN);
    input_read_line("请输入联系电话: ", room->phone, PHONE_LEN);
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
            printf("初始化失败：内存不足。\n");
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
        printf("当前没有房间信息。\n");
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
        printf("当前没有空闲房间。\n");
    }
}

void hotel_reserve(Room *head) {
    int roomNumber = input_room_number();
    Room *room;

    if (roomNumber < 0) {
        printf("房号输入无效。\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("未找到该房间。\n");
        return;
    }

    if (room->status != ROOM_AVAILABLE) {
        printf("该房间当前不是空闲状态。\n");
        return;
    }

    input_guest_info(room);
    if (room->stayDays <= 0 || room->guestName[0] == '\0') {
        reset_guest_info(room);
        printf("预定失败：信息输入不完整。\n");
        return;
    }

    room->status = ROOM_RESERVED;
    printf("预定成功：房号 %d，预计费用 %.2f\n", room->roomNumber, room->price * room->stayDays);
}

void hotel_cancel_reservation(Room *head) {
    int roomNumber = input_room_number();
    Room *room;

    if (roomNumber < 0) {
        printf("房号输入无效。\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("未找到该房间。\n");
        return;
    }

    if (room->status != ROOM_RESERVED) {
        printf("该房间不是预定状态，无法取消。\n");
        return;
    }

    room->status = ROOM_AVAILABLE;
    reset_guest_info(room);
    printf("已取消预定。\n");
}

void hotel_check_in(Room *head) {
    int roomNumber = input_room_number();
    Room *room;

    if (roomNumber < 0) {
        printf("房号输入无效。\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("未找到该房间。\n");
        return;
    }

    if (room->status == ROOM_OCCUPIED) {
        printf("该房间已入住。\n");
        return;
    }

    if (room->status == ROOM_AVAILABLE) {
        printf("该房间尚未预定，将按散客入住处理。\n");
        input_guest_info(room);
        if (room->stayDays <= 0 || room->guestName[0] == '\0') {
            reset_guest_info(room);
            printf("入住失败：信息输入不完整。\n");
            return;
        }
    }

    room->status = ROOM_OCCUPIED;
    printf("入住成功：房号 %d\n", room->roomNumber);
}

void hotel_check_out(Room *head) {
    int roomNumber = input_room_number();
    Room *room;
    double bill;

    if (roomNumber < 0) {
        printf("房号输入无效。\n");
        return;
    }

    room = find_room(head, roomNumber);
    if (room == NULL) {
        printf("未找到该房间。\n");
        return;
    }

    if (room->status != ROOM_OCCUPIED) {
        printf("该房间未处于入住状态。\n");
        return;
    }

    bill = room->price * room->stayDays;
    printf("退房成功，账单金额：%.2f\n", bill);
    room->status = ROOM_AVAILABLE;
    reset_guest_info(room);
}

void hotel_query_by_guest(Room *head) {
    char name[NAME_LEN];
    int found = 0;

    if (!input_read_line("请输入客人姓名关键字: ", name, NAME_LEN) || name[0] == '\0') {
        printf("输入为空。\n");
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
        printf("未找到相关客人记录。\n");
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

    printf("空闲:%d 预定:%d 入住:%d\n", available, reserved, occupied);
    printf("当前预估营业额：%.2f\n", expectedRevenue);
}

void hotel_add_room(Room **head) {
    int number;
    int type;
    double price;
    Room *newRoom;

    if (!input_read_int("请输入新房号: ", &number)) {
        printf("输入无效。\n");
        return;
    }

    if (find_room(*head, number) != NULL) {
        printf("房号已存在。\n");
        return;
    }

    if (!input_read_int("请输入房型(1/2/3): ", &type)) {
        printf("输入无效。\n");
        return;
    }

    if (!input_read_double("请输入每晚价格: ", &price) || price <= 0) {
        printf("价格输入无效。\n");
        return;
    }

    newRoom = create_room(number, type, price);
    if (newRoom == NULL) {
        printf("添加失败：内存不足。\n");
        return;
    }

    append_room(head, newRoom);
    printf("房间添加成功。\n");
}

void hotel_remove_room(Room **head) {
    int number = input_room_number();
    Room *current = *head;
    Room *prev = NULL;

    if (number < 0) {
        printf("房号输入无效。\n");
        return;
    }

    while (current != NULL && current->roomNumber != number) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("未找到该房间。\n");
        return;
    }

    if (current->status == ROOM_OCCUPIED) {
        printf("该房间已入住，不能删除。\n");
        return;
    }

    if (prev == NULL) {
        *head = current->next;
    } else {
        prev->next = current->next;
    }

    free(current);
    printf("房间删除成功。\n");
}

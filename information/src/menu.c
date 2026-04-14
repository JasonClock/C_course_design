//
// Created by 18032 on 2026/4/14.
//

#include "menu.h"
#include "input.h"

#include <stdio.h>

void menu_show(void) {
    printf("\n========== 酒店房间预定管理系统 ==========\n");
    printf("1.查看所有房间\n");
    printf("2.查看空闲房间\n");
    printf("3.预定房间\n");
    printf("4.取消预定\n");
    printf("5.入住办理\n");
    printf("6.退房结算\n");
    printf("7.按客人姓名查询\n");
    printf("8.统计信息\n");
    printf("9.添加房间\n");
    printf("10.删除房间\n");
    printf("0.退出系统\n");
}

int menu_read_choice(void) {
    int choice;
    if (!input_read_int("请选择操作: ", &choice)) {
        return -1;
    }
    return choice;
}

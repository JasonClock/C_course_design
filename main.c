#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hotel.h"
#include "input.h"
#include "hotel_storage.h"
#include "menu.h"

/**
 * 功能：去掉命令字符串首尾空白字符，统一后续命令匹配行为。
 * @param text 待处理的命令字符串缓冲区（原地修改）。
 */
static void trim_command(char *text) {
    char *start;
    char *end;

    start = text;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        ++start;
    }

    if (start != text) {
        memmove(text, start, strlen(start) + 1);
    }

    end = text + strlen(text);
    while (end > text && isspace((unsigned char)*(end - 1))) {
        --end;
    }
    *end = '\0';
}

/**
 * 功能：将命令字符串转换为小写，避免大小写导致的命令识别差异。
 * @param text 待转换的命令字符串缓冲区（原地修改）。
 */
static void to_lowercase(char *text) {
    char *p;

    for (p = text; *p != '\0'; ++p) {
        *p = (char)tolower((unsigned char)*p);
    }
}

/**
 * 功能：将文本命令解析为菜单数字选项。
 * @param command 用户输入的命令文本。
 * @param choice 解析成功后写入的菜单选项值。
 * @return 解析成功返回1；失败返回0。
 */
static int parse_menu_choice(const char *command, int *choice) {
    char *endPtr;
    long value;

    if (command[0] == '\0') {
        return 0;
    }

    value = strtol(command, &endPtr, 10);
    if (*endPtr != '\0') {
        return 0;
    }

    *choice = (int)value;
    return 1;
}

/**
 * 功能：程序主入口，负责启动加载、角色切换命令处理、菜单分发与退出保存。
 * 说明：该函数无参数。
 * @return 正常退出返回0；初始化失败返回1。
 */
int main(void) {
    UserRole role = USER_CUSTOMER;
    const UserOps *ops;
    Room *roomList = NULL;
    char command[64];
    int choice;
    int result;

    /* 启动时优先加载持久化数据，加载失败时回退到默认房间初始化。 */
    if (!hotel_storage_load(&roomList, HOTEL_DATA_DIR)) {
        hotel_init(&roomList);
    }

    ops = role_get_ops(role);

    if (ops == NULL) {
        hotel_free(roomList);
        printf("Role initialization failed.\n");
        return 1;
    }

    printf("Startup role: %s. Type sudo to switch to Administrator.\n", ops->name);

    while (1) {
        menu_show(role);
        if (!input_read_line("Enter command: ", command, (int)sizeof(command))) {
            printf("Invalid input.\n");
            continue;
        }

        trim_command(command);
        to_lowercase(command);

        if (command[0] == '\0') {
            printf("Please enter a command.\n");
            continue;
        }

        /* 普通用户通过 sudo 命令切换为管理员身份。 */
        if (role == USER_CUSTOMER && strcmp(command, "sudo") == 0) {
            role = USER_ADMIN;
            ops = role_get_ops(role);
            printf("Switched to %s.\n", ops->name);
            continue;
        }

        /* 管理员通过 user 命令切换回普通用户身份。 */
        if (role == USER_ADMIN && strcmp(command, "user") == 0) {
            role = USER_CUSTOMER;
            ops = role_get_ops(role);
            printf("Switched to %s.\n", ops->name);
            continue;
        }

        if (strcmp(command, "exit") == 0) {
            choice = 0;
        } else if (!parse_menu_choice(command, &choice)) {
            printf("Unknown command. Use a menu number, sudo, user, or exit.\n");
            continue;
        }

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

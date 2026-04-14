#include "hotel_storage.h"

#include <direct.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ROOMS_FILE_NAME "rooms.db"
#define RESERVATIONS_FILE_NAME "reservations.db"
#define SNAPSHOT_PREFIX "#SNAPSHOT "

static int make_path(char *outPath, size_t size, const char *dir, const char *fileName);

static int path_exists(const char *path) {
    struct _stat info;
    if (path == NULL) {
        return 0;
    }
    return _stat(path, &info) == 0;
}

static int path_is_dir(const char *path) {
    struct _stat info;
    if (path == NULL) {
        return 0;
    }
    if (_stat(path, &info) != 0) {
        return 0;
    }
    return (info.st_mode & _S_IFDIR) != 0;
}

static int make_candidate_dir(char *outPath, size_t size, const char *prefix, const char *dataDir) {
    int written;
    if (prefix == NULL || prefix[0] == '\0') {
        written = snprintf(outPath, size, "%s", dataDir);
    } else {
        written = snprintf(outPath, size, "%s/%s", prefix, dataDir);
    }
    return written > 0 && (size_t)written < size;
}

static int resolve_data_dir(char *outPath, size_t size, const char *dataDir) {
    const char *prefixes[] = {"..", "../..", ""};
    int bestScore = -1;
    size_t i;

    if (outPath == NULL || size == 0 || dataDir == NULL) {
        return 0;
    }

    for (i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
        char candidate[260];
        char roomPath[260];
        char reservationPath[260];
        int score = 0;

        if (!make_candidate_dir(candidate, sizeof(candidate), prefixes[i], dataDir)) {
            continue;
        }

        if (!make_path(roomPath, sizeof(roomPath), candidate, ROOMS_FILE_NAME) ||
            !make_path(reservationPath, sizeof(reservationPath), candidate, RESERVATIONS_FILE_NAME)) {
            continue;
        }

        if (path_is_dir(candidate)) {
            score += 1;
        }
        if (path_exists(roomPath)) {
            score += 4;
        }
        if (path_exists(reservationPath)) {
            score += 2;
        }

        if (score > bestScore) {
            int written = snprintf(outPath, size, "%s", candidate);
            if (written > 0 && (size_t)written < size) {
                bestScore = score;
            }
        }
    }

    if (bestScore < 0) {
        int written = snprintf(outPath, size, "%s", dataDir);
        return written > 0 && (size_t)written < size;
    }

    return 1;
}

static int make_path(char *outPath, size_t size, const char *dir, const char *fileName) {
    int written = snprintf(outPath, size, "%s/%s", dir, fileName);
    return written > 0 && (size_t)written < size;
}

static int ensure_data_dir(const char *dataDir) {
    if (_mkdir(dataDir) == 0) {
        return 1;
    }
    return errno == EEXIST;
}

static Room *create_room_node(int roomNumber, int roomType, double price) {
    Room *room = (Room *)malloc(sizeof(Room));
    if (room == NULL) {
        return NULL;
    }

    room->roomNumber = roomNumber;
    room->roomType = roomType;
    room->price = price;
    room->reservations = NULL;
    room->next = NULL;
    return room;
}

static Reservation *create_reservation_node(Date startDate,
                                            Date endDate,
                                            int checkedIn,
                                            int paymentStatus,
                                            time_t lockTime,
                                            const char *guestName,
                                            const char *phone) {
    Reservation *reservation = (Reservation *)malloc(sizeof(Reservation));
    if (reservation == NULL) {
        return NULL;
    }

    reservation->startDate = startDate;
    reservation->endDate = endDate;
    reservation->checkedIn = checkedIn;
    reservation->paymentStatus = paymentStatus == PAYMENT_PAID ? PAYMENT_PAID : PAYMENT_LOCKED;
    reservation->lockTime = lockTime;
    strncpy(reservation->guestName, guestName, NAME_LEN - 1);
    reservation->guestName[NAME_LEN - 1] = '\0';
    strncpy(reservation->phone, phone, PHONE_LEN - 1);
    reservation->phone[PHONE_LEN - 1] = '\0';
    reservation->next = NULL;
    return reservation;
}

static void free_reservations(Reservation *head) {
    Reservation *next;
    while (head != NULL) {
        next = head->next;
        free(head);
        head = next;
    }
}

static void free_rooms(Room *head) {
    Room *next;
    while (head != NULL) {
        next = head->next;
        free_reservations(head->reservations);
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

static Room *find_room(Room *head, int roomNumber) {
    while (head != NULL) {
        if (head->roomNumber == roomNumber) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

static void append_reservation(Room *room, Reservation *node) {
    Reservation *current;
    if (room->reservations == NULL) {
        room->reservations = node;
        return;
    }

    current = room->reservations;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = node;
}

static int is_snapshot_line(const char *line) {
    return strncmp(line, SNAPSHOT_PREFIX, strlen(SNAPSHOT_PREFIX)) == 0;
}

static int print_file_with_title(const char *title, const char *filePath) {
    FILE *file = fopen(filePath, "r");
    char line[512];

    printf("\n----- %s -----\n", title);
    if (file == NULL) {
        printf("(file not found)\n");
        return 1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    fclose(file);
    return 1;
}

int hotel_storage_load(Room **head, const char *dataDir) {
    char resolvedDir[260];
    char roomPath[260];
    char reservationPath[260];
    FILE *roomFile;
    FILE *reservationFile;
    char line[512];
    Room *loadedRooms = NULL;

    if (head == NULL || dataDir == NULL) {
        return 0;
    }

    *head = NULL;

    if (!resolve_data_dir(resolvedDir, sizeof(resolvedDir), dataDir)) {
        return 0;
    }

    if (!make_path(roomPath, sizeof(roomPath), resolvedDir, ROOMS_FILE_NAME) ||
        !make_path(reservationPath, sizeof(reservationPath), resolvedDir, RESERVATIONS_FILE_NAME)) {
        return 0;
    }

    roomFile = fopen(roomPath, "r");
    if (roomFile == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), roomFile) != NULL) {
        int roomNumber;
        int roomType;
        double price;
        Room *room;

        if (is_snapshot_line(line)) {
            free_rooms(loadedRooms);
            loadedRooms = NULL;
            continue;
        }

        if (line[0] == '#') {
            continue;
        }

        if (sscanf(line, "%d|%d|%lf", &roomNumber, &roomType, &price) != 3) {
            continue;
        }

        room = create_room_node(roomNumber, roomType, price);
        if (room == NULL) {
            fclose(roomFile);
            free_rooms(loadedRooms);
            return 0;
        }
        append_room(&loadedRooms, room);
    }
    fclose(roomFile);

    if (loadedRooms == NULL) {
        return 0;
    }

    reservationFile = fopen(reservationPath, "r");
    if (reservationFile == NULL) {
        *head = loadedRooms;
        return 1;
    }

    /* Only load reservation records from the latest snapshot block. */
    while (fgets(line, sizeof(line), reservationFile) != NULL) {
        if (is_snapshot_line(line)) {
            Room *cursor = loadedRooms;
            while (cursor != NULL) {
                free_reservations(cursor->reservations);
                cursor->reservations = NULL;
                cursor = cursor->next;
            }
            continue;
        }

        if (line[0] == '#') {
            continue;
        }

        {
            int roomNumber;
            Date startDate;
            Date endDate;
            int checkedIn;
            int paymentStatus;
            long long lockTimeRaw;
            char guestName[NAME_LEN];
            char phone[PHONE_LEN];
            Room *room;
            Reservation *reservation;

            if (sscanf(line,
                       "%d|%d|%d|%d|%d|%d|%d|%d|%d|%lld|%49[^|]|%19[^\n]",
                       &roomNumber,
                       &startDate.year,
                       &startDate.month,
                       &startDate.day,
                       &endDate.year,
                       &endDate.month,
                       &endDate.day,
                       &checkedIn,
                       &paymentStatus,
                       &lockTimeRaw,
                       guestName,
                       phone) != 12) {
                continue;
            }

            room = find_room(loadedRooms, roomNumber);
            if (room == NULL) {
                continue;
            }

            reservation = create_reservation_node(startDate,
                                                  endDate,
                                                  checkedIn,
                                                  paymentStatus,
                                                  (time_t)lockTimeRaw,
                                                  guestName,
                                                  phone);
            if (reservation == NULL) {
                fclose(reservationFile);
                free_rooms(loadedRooms);
                return 0;
            }
            append_reservation(room, reservation);
        }
    }

    fclose(reservationFile);
    *head = loadedRooms;
    return 1;
}

int hotel_storage_save(Room *head, const char *dataDir) {
    char resolvedDir[260];
    char roomPath[260];
    char reservationPath[260];
    FILE *roomFile;
    FILE *reservationFile;
    long long snapshotId;

    if (head == NULL || dataDir == NULL) {
        return 0;
    }

    if (!resolve_data_dir(resolvedDir, sizeof(resolvedDir), dataDir)) {
        return 0;
    }

    if (!ensure_data_dir(resolvedDir)) {
        return 0;
    }

    if (!make_path(roomPath, sizeof(roomPath), resolvedDir, ROOMS_FILE_NAME) ||
        !make_path(reservationPath, sizeof(reservationPath), resolvedDir, RESERVATIONS_FILE_NAME)) {
        return 0;
    }

    roomFile = fopen(roomPath, "a");
    if (roomFile == NULL) {
        return 0;
    }

    reservationFile = fopen(reservationPath, "a");
    if (reservationFile == NULL) {
        fclose(roomFile);
        return 0;
    }

    snapshotId = (long long)time(NULL);
    fprintf(roomFile, "%s%lld\n", SNAPSHOT_PREFIX, snapshotId);
    fprintf(reservationFile, "%s%lld\n", SNAPSHOT_PREFIX, snapshotId);

    while (head != NULL) {
        Reservation *res;

        fprintf(roomFile, "%d|%d|%.2f\n", head->roomNumber, head->roomType, head->price);

        res = head->reservations;
        while (res != NULL) {
            fprintf(reservationFile,
                    "%d|%d|%d|%d|%d|%d|%d|%d|%d|%lld|%s|%s\n",
                    head->roomNumber,
                    res->startDate.year,
                    res->startDate.month,
                    res->startDate.day,
                    res->endDate.year,
                    res->endDate.month,
                    res->endDate.day,
                    res->checkedIn,
                    (int)res->paymentStatus,
                    (long long)res->lockTime,
                    res->guestName,
                    res->phone);
            res = res->next;
        }

        head = head->next;
    }

    fclose(roomFile);
    fclose(reservationFile);
    return 1;
}

int hotel_storage_view(const char *dataDir) {
    char resolvedDir[260];
    char roomPath[260];
    char reservationPath[260];

    if (dataDir == NULL) {
        return 0;
    }

    if (!resolve_data_dir(resolvedDir, sizeof(resolvedDir), dataDir)) {
        return 0;
    }

    if (!make_path(roomPath, sizeof(roomPath), resolvedDir, ROOMS_FILE_NAME) ||
        !make_path(reservationPath, sizeof(reservationPath), resolvedDir, RESERVATIONS_FILE_NAME)) {
        return 0;
    }

    printf("\n========== data_house content ==========%s", "\n");
    print_file_with_title("rooms.db", roomPath);
    print_file_with_title("reservations.db", reservationPath);
    printf("----------------------------------------\n");
    return 1;
}

int hotel_storage_reset(Room **head, const char *dataDir) {
    char resolvedDir[260];
    char roomPath[260];
    char reservationPath[260];
    FILE *roomFile;
    FILE *reservationFile;

    if (head == NULL || dataDir == NULL) {
        return 0;
    }

    if (!resolve_data_dir(resolvedDir, sizeof(resolvedDir), dataDir)) {
        return 0;
    }

    if (!ensure_data_dir(resolvedDir)) {
        return 0;
    }

    if (!make_path(roomPath, sizeof(roomPath), resolvedDir, ROOMS_FILE_NAME) ||
        !make_path(reservationPath, sizeof(reservationPath), resolvedDir, RESERVATIONS_FILE_NAME)) {
        return 0;
    }

    roomFile = fopen(roomPath, "w");
    if (roomFile == NULL) {
        return 0;
    }
    fclose(roomFile);

    reservationFile = fopen(reservationPath, "w");
    if (reservationFile == NULL) {
        return 0;
    }
    fclose(reservationFile);

    if (*head != NULL) {
        hotel_free(*head);
        *head = NULL;
    }

    hotel_init(head);
    return hotel_storage_save(*head, resolvedDir);
}


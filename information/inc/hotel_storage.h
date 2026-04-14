#ifndef C_HOTEL_STORAGE_H
#define C_HOTEL_STORAGE_H

#include "hotel.h"

int hotel_storage_load(Room **head, const char *dataDir);
int hotel_storage_save(Room *head, const char *dataDir);
int hotel_storage_view(const char *dataDir);
int hotel_storage_reset(Room **head, const char *dataDir);

#endif //C_HOTEL_STORAGE_H


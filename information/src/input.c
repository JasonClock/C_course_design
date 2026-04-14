//
// Created by 18032 on 2026/4/14.
//

#include "input.h"

#include <stdio.h>
#include <string.h>

void input_clear_buffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

int input_read_int(const char *prompt, int *outValue) {
    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (scanf("%d", outValue) != 1) {
        input_clear_buffer();
        return 0;
    }

    input_clear_buffer();
    return 1;
}

int input_read_double(const char *prompt, double *outValue) {
    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (scanf("%lf", outValue) != 1) {
        input_clear_buffer();
        return 0;
    }

    input_clear_buffer();
    return 1;
}

int input_read_line(const char *prompt, char *buffer, int size) {
    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (fgets(buffer, size, stdin) == NULL) {
        if (size > 0) {
            buffer[0] = '\0';
        }
        return 0;
    }

    buffer[strcspn(buffer, "\n")] = '\0';
    return 1;
}

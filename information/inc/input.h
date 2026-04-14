//
// Created by 18032 on 2026/4/14.
//

#ifndef C_INPUT_H
#define C_INPUT_H

int input_read_int(const char *prompt, int *outValue);
int input_read_double(const char *prompt, double *outValue);
int input_read_line(const char *prompt, char *buffer, int size);
void input_clear_buffer(void);

#endif //C_INPUT_H

//
// Created by ellepsis on 03.05.15.
//

#ifndef LAB4_LINESNEW_H
#define LAB4_LINESNEW_H
#include <unistd.h>

typedef struct lines_llist{
    char *buff;
    size_t line_number;
    size_t buff_lenght;
    char isForward;
    struct lines_llist *previous;
    struct lines_llist *next;
} lines_llist;

size_t write_n_lines(size_t lines_count, char isForward);
void initialize_lines_ll();
struct winsize get_term_size();
void initialize_buffer_info();
#endif //LAB4_LINESNEW_H

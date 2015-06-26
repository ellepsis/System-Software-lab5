#ifndef LAB4_LINES_H
#define LAB4_LINES_H

#include "term_in.h"
#include "termio.h"

#define LINE_BUFFER_MAX_SIZE 128*4
#define LINES_LLCOUNT 1000

typedef struct skip_lines_status{
    size_t count_lines;
    size_t skipped_bytes_count;
    size_t current_line;
}skip_lines_status;

typedef struct lines_llist{
    char *buff;
    size_t line_number;
    size_t buff_lenght;
    struct lines_llist *previous;
    struct lines_llist *next;
} lines_llist;

lines_llist _lines_ll[LINES_LLCOUNT];

struct winsize get_term_size();
skip_lines_status get_draw_bytes_count_forward(struct winsize size, size_t lines, buffer_info *buff_info,
                                               off_t start_offset, lines_llist *lines_ll, ssize_t current_line);
void initialize_lines_ll();
char is_last_writed_symbol_the_new_line();

#endif

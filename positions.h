#ifndef LAB4_POSITIONS_H
#define LAB4_POSITIONS_H

#include <unistd.h>

typedef struct positions_s{
    off_t start;
    off_t end;
    size_t lines_count;
    size_t number;
}positions_s;

typedef struct position_s{
    char isEnd;
    off_t offset;
}position_s;

size_t write_lines_forward(size_t count_liens);
size_t write_lines_backward(size_t count_lines);
void add_new_positions(size_t count);
size_t fill_positions(size_t count, size_t start_count, off_t start_offset);
void clear_pipe_end();
void initialize_buffer_info();

#endif

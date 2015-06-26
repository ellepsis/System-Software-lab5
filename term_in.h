#ifndef TERM_IN_H
#define TERM_IN_H

#define BUFFER_STANDART_SIZE 1024*30
#include <unistd.h>
typedef struct {
    char *buff;
    size_t buff_len;
    size_t buff_offset;
    off_t file_start_offset;
    char is_end; /*End of file*/
} buffer_info;

ssize_t read_to_buffer_forward(buffer_info *buff_info, size_t bytes, size_t buffer_offset);
void set_file_offset(off_t offset);
char is_a_regular();
int set_next_file();
void set_files_info(char **paths, int count);
char is_end_of_pipe();
int get_terminal_file_descriptor();

#endif

#ifndef H_TERM_OUT
#define H_TERM_OUT

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "term_out.h"
#include "lines.h"

void set_term_row_count(char *lines_count);
struct winsize get_term_size();
ssize_t write_out(char *buff, size_t bytes_count);
size_t get_line_ll_index(long long offset, size_t current_line);
size_t write_from_lines_ll(lines_llist *start, size_t count);
#endif 

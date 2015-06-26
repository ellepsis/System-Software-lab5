#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include "more.h"
#include "term_out.h"

#define MIN_TERM_SIZE 3
unsigned short term_row_count;
char need_use_term_row_count;

struct winsize get_term_size() {
    struct winsize w;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w)<0) print_error("Get Terminal Size", 5, errno);
    /*DEBUG
    w.ws_row = 56;
    w.ws_col = 106;
    /*ENDDEBUG*/
    if (need_use_term_row_count) w.ws_row = term_row_count;
    if( w.ws_row < MIN_TERM_SIZE || w.ws_col < MIN_TERM_SIZE) print_error("Small terminal", 5, errno);
    if (w.ws_row > LINES_LLCOUNT) print_error("Invalid terminal size: so large", 17, errno);
    if (w.ws_col > LINE_BUFFER_MAX_SIZE) print_error("Invalid terminal size: so large", 17, errno);
    return w;
}

void set_term_row_count(char* lines_count) {
    int count = 0;
    char ch;
    while ((ch =*lines_count++)) {
        if (isdigit(ch)) {
            count =(count * 10 + ch - '0') < count ? count : (count * 10 + ch - '0');
        }
        else {
            print_error("Argument is not a integer", 19, errno);
        }
    }
    if (count < MIN_TERM_SIZE) print_error("Invalid terminal size: so small", 16, errno);
    if (count > LINES_LLCOUNT) print_error("Invalid terminal size: so large", 17, errno);
    term_row_count = (unsigned short) count;
    need_use_term_row_count = 1;
}

/*Write to terminal*/
ssize_t write_out(char *buff, size_t bytes_count){
    ssize_t writed_count;
    if ((writed_count = write(STDOUT_FILENO, buff, bytes_count)) < 0) print_error("Can't redraw", 6, errno);
    return writed_count;
}

/*Write n lines from lines linked list to terminal
 * May be can be accelerated by concatenating all buffers and after that write it*/
size_t write_from_lines_ll(lines_llist *start, size_t count){
    size_t  i, write_return = 0;
    lines_llist *next;
    next = start;
    for (i = 0; i < count; i++){
        write_return += write_out(next->buff, next->buff_lenght);
        next = next->next;
    }
    return write_return;
}
    struct winsize ws;
/*Get index at line_ll who's position at positive or negative offset of current*/
size_t get_line_ll_index(long long offset, size_t current_line){
    offset = offset%LINES_LLCOUNT;
    if ((ssize_t)current_line + offset < 0) return current_line+(LINES_LLCOUNT-1+offset);
    if (current_line + offset < LINES_LLCOUNT) return current_line+offset;
    else return current_line-LINES_LLCOUNT-1+offset;
}

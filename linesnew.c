//
// Created by ellepsis on 03.05.15.
//

#include <sys/ioctl.h>
#include <errno.h>
#include "term_in.h"
#include "more.h"
#include "linesnew.h"
#include "term_out.h"
#include "malloc.h"

#define LINES_LLCOUNT 1000

lines_llist _lines_ll[LINES_LLCOUNT];
buffer_info _buff_info;
/*---------GLOBAL--------*/
size_t _current_line = 0;
size_t last_line_size;
char prewious_is_backward=0;
/*-------END GLOBAL------*/

typedef struct skip_lines_status{
    size_t count_lines;
    size_t skipped_bytes_count;
    ssize_t  current_line;
}skip_lines_status;

struct winsize get_term_size() {
    struct winsize w;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w)<0) print_error("Get Terminal Size", errno, 5);
    /*DEBUG*/
    w.ws_row = 29;
    w.ws_col = 30;
    /*ENDDEBUG*/
    return w;
}

inline size_t term_char_count_in_char(char ch, size_t term_length){
    switch (ch) {
        case '\n':
            return term_length;
        case '\t':
            return 4;
        default:
            return 1;
    }
}

skip_lines_status get_draw_bytes_count_forward(struct winsize size, size_t_64 lines, buffer_info *buff_info,
                                               lines_llist *lines_ll, ssize_t current_line){
    register size_t lines_count = 0, char_count = 0;
    register size_t local_offset = 0;
    register size_t line_offset = 0;
    skip_lines_status c_status;
    c_status.skipped_bytes_count=0;
    local_offset = buff_info->buff_offset;
    while(1){
        while (1) {
            if (buff_info->buff_len == 0) break;
            lines_ll[current_line].buff[line_offset++] = buff_info->buff[local_offset];
            char_count+=term_char_count_in_char(buff_info->buff[local_offset], size.ws_col);
            c_status.skipped_bytes_count++;
            /*if char count bigger than terminal char count in line*/
            if (char_count >= size.ws_col) {
                last_line_size = char_count;
                char_count = 0;
                lines_count++;
                lines_ll[current_line].buff[line_offset] = '\0';
                lines_ll[current_line].buff_lenght = line_offset;
                lines_ll[current_line].line_number = lines_ll[current_line].previous->line_number + 1;
                lines_ll[current_line].isForward = 1;
                ++current_line < LINES_LLCOUNT ? current_line : (current_line = 0);
                 line_offset = 0;
            }
            /*return if contain all requested lines*/
            if (lines_count == lines) {
                buff_info->buff_offset = ++local_offset;
                c_status.count_lines = lines_count;
                c_status.current_line = current_line;
                return c_status;
            };
            /*if go forward increment offset, else decrement*/
            if (local_offset >= buff_info->buff_len) break;
            else local_offset++;
        }
        if (!buff_info->is_end){ /*If data exitst - read forward*/
            ssize_t read_bytes_count;
            read_bytes_count = read_to_buffer_forward(buff_info, BUFFER_STANDART_SIZE, 0);
            buff_info->buff_offset = 0;
            local_offset = 0;
            buff_info->buff_len = (size_t)read_bytes_count;
            continue;
        }

        /* End of data mean end of file */
        lines_ll[current_line].buff_lenght = line_offset;
        lines_ll[current_line].line_number = lines_ll[current_line].previous->line_number+1;
        ++current_line<LINES_LLCOUNT?current_line:(current_line = 0);
        lines_ll[current_line].isForward = 1;
        buff_info->buff_offset = local_offset+1;
        c_status.count_lines = lines_count;
        c_status.current_line = current_line;
        return c_status;
    }
}

/*Write n lines from lines linked list to terminal
 * May be can be accelerated by concatenating all buffers and after that write it*/
size_t write_from_lines_ll(lines_llist *start, size_t count){
    size_t  i, write_return = 0;
    lines_llist *next;
    next = start;
    for (i = 0; i < count; i++){
        if (next->isForward) write_return += write_out(next->buff, next->buff_lenght);
        else write_return += write_out(next->buff+LINE_BUFFER_MAX_SIZE-1-next->buff_lenght, next->buff_lenght);
        next = next->next;
    }
    return write_return;
}

/*Get index at line_ll who's position at positive or negative offset of current*/
size_t get_line_ll_index(int offset, ssize_t current_line){
    offset = offset%LINES_LLCOUNT;
    if (current_line + offset < 0) return current_line+(LINES_LLCOUNT-1+offset);
    if (current_line + offset < LINES_LLCOUNT) return current_line+offset;
    else return current_line-LINES_LLCOUNT-1+offset;
}

size_t write_n_lines(size_t lines_count, char isForward) {
    struct winsize current_size;
    size_t ret_lines_count;
    skip_lines_status skip_status;
    current_size = get_term_size();
    current_size.ws_row--; /*One last string for more or filename*/
    current_size.ws_col--;
    if (!isForward) lines_count+=current_size.ws_row;
    /*Check available lines count*/
    if (isForward)
        skip_status = get_draw_bytes_count_forward(current_size, lines_count, &_buff_info, _lines_ll, _current_line);
    else {
        skip_status = get_draw_bytes_count_backward(current_size, lines_count, &_buff_info, _lines_ll, _current_line, last_line_size);
        lines_count -= current_size.ws_row;
        skip_status = get_draw_bytes_count_forward(current_size, lines_count, &_buff_info, _lines_ll, --_current_line >= 0 ? _current_line : (_current_line = LINES_LLCOUNT-1));
    }
    _current_line = skip_status.current_line;
    /*if Readed bytes count lower than needed lines count, than write it*/
    if (skip_status.count_lines < current_size.ws_row) {
        write_from_lines_ll(&_lines_ll[get_line_ll_index(-((int)skip_status.count_lines), skip_status.current_line)], skip_status.count_lines);
        ret_lines_count = skip_status.count_lines;
    }
        /*if Readed bytes count more than needed lines count, than write only last lines*/
    else {
        write_from_lines_ll(&_lines_ll[get_line_ll_index(-((int) current_size.ws_row), skip_status.current_line)], current_size.ws_row);
        ret_lines_count = current_size.ws_row;
    }
    return ret_lines_count;
}

void initialize_buffer_info(){
    ssize_t readed_bytes_count;
    _buff_info.buff = (char*) malloc(BUFFER_STANDART_SIZE*sizeof(char));
    _buff_info.buff_len = BUFFER_STANDART_SIZE;
    readed_bytes_count = read_to_buffer_forward(&_buff_info, BUFFER_STANDART_SIZE, 0);
    _buff_info.buff_offset = 0;
    _buff_info.buff_len = (size_t)readed_bytes_count;
}

/*Link lines linked list*/
void initialize_lines_ll(){
    int i;
    _lines_ll[0].buff = (char*) malloc((LINES_LLCOUNT+1)*LINE_BUFFER_MAX_SIZE*sizeof(char));
    for (i = 1; i < LINES_LLCOUNT-1; i++){
        _lines_ll[i].buff = _lines_ll[0].buff+(i*LINE_BUFFER_MAX_SIZE);
        _lines_ll[i].previous = &_lines_ll[i-1];
        _lines_ll[i].next = &_lines_ll[i+1];
        _lines_ll[i].line_number = (size_t)i;
        _lines_ll[i].buff_lenght = 0;
        _lines_ll[i].isForward = 2;
    }
    _lines_ll[0].next = &_lines_ll[1];
    _lines_ll[0].previous = &_lines_ll[LINES_LLCOUNT-1];
    _lines_ll[LINES_LLCOUNT-1].next = &_lines_ll[0];
    _lines_ll[LINES_LLCOUNT-1].previous = &_lines_ll[LINES_LLCOUNT-2];
    _lines_ll[LINES_LLCOUNT-1].buff = _lines_ll[0].buff+((LINES_LLCOUNT-1)*LINE_BUFFER_MAX_SIZE);
    _lines_ll[0].isForward = 2;
    _lines_ll[LINES_LLCOUNT-1].isForward = 2;
    _lines_ll[0].previous->line_number = -1;
}
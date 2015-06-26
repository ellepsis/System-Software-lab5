#include <sys/ioctl.h>
#include <errno.h>
#include <malloc.h>
#include "term_in.h"
#include "more.h"
#include "lines.h"
#define TAB_SIZE 8

char is_last_symbol_the_new_line = 0;

void check_buffer(buffer_info *buff_info, off_t start_offset){
    if ((ssize_t)buff_info->file_start_offset > start_offset || buff_info->file_start_offset+(ssize_t)buff_info->buff_len <= start_offset) {
        buff_info->buff_len = 0;
        set_file_offset(start_offset);
        buff_info->is_end = 0;
    }
    else buff_info->buff_offset = (size_t)(start_offset - buff_info->file_start_offset);
}

inline size_t term_char_count_in_char(char ch, size_t term_length, size_t char_count){
    switch (ch) {
        case '\n':
            return term_length;
        case '\t':
            return char_count%TAB_SIZE?TAB_SIZE-char_count%TAB_SIZE:TAB_SIZE;
        default:
            return 1;
    }
}

skip_lines_status get_draw_bytes_count_forward(struct winsize size, size_t lines, buffer_info *buff_info,
                                               off_t start_offset, lines_llist *lines_ll, ssize_t current_line) {
    register size_t lines_count = 0, char_count = 0;
    register size_t local_offset = 0, line_offset = 0;
    skip_lines_status c_status;
    c_status.skipped_bytes_count = 0;
    check_buffer(buff_info, start_offset);
    local_offset = buff_info->buff_offset;
    while (1) {
        while (1) {
            if (buff_info->buff_len == 0) break;
            if (buff_info->buff[local_offset] == '\0') lines_ll[current_line].buff[line_offset++] = ' ';
            else lines_ll[current_line].buff[line_offset++] = buff_info->buff[local_offset];
            char_count += term_char_count_in_char(buff_info->buff[local_offset], size.ws_col, char_count);
            c_status.skipped_bytes_count++;
            /*if char count bigger than terminal char count in line*/
            if (char_count >= size.ws_col) {
                char_count = 0;
                lines_count++;
                if (buff_info->buff[local_offset] == '\n') is_last_symbol_the_new_line = 1;
                else is_last_symbol_the_new_line = 0;
                lines_ll[current_line].buff_lenght = line_offset;
                lines_ll[current_line].line_number = lines_ll[current_line].previous->line_number + 1;
                ++current_line < LINES_LLCOUNT ? current_line : (current_line = 0);
                line_offset = 0;
            }
            /*return if contain all requested lines*/
            if (lines_count == lines) {
                c_status.count_lines = lines_count;
                c_status.current_line = (size_t)current_line;
                return c_status;
            };
            /*if go forward increment offset, else decrement*/
            if (local_offset >= buff_info->buff_len-1)
                break;
            else local_offset++;
        }
        if (!buff_info->is_end) { /*If data exitst - read forward*/
            ssize_t read_bytes_count;
            read_bytes_count = read_to_buffer_forward(buff_info, BUFFER_STANDART_SIZE, 0);
            buff_info->buff_offset = 0;
            local_offset = 0;
            buff_info->buff_len = (size_t) read_bytes_count;
            continue;
        }
        /* End of data mean end of file */
        lines_ll[current_line].buff_lenght = line_offset;
        lines_ll[current_line].line_number = lines_ll[current_line].previous->line_number+1;
        current_line<LINES_LLCOUNT?current_line:(current_line = 0);
        buff_info->buff_offset = local_offset+1;
        c_status.count_lines = lines_count;
        c_status.current_line = (size_t)current_line;
        return c_status;
    }
}

char is_last_writed_symbol_the_new_line(){
    return is_last_symbol_the_new_line;
}

/*Link lines linked list*/
void initialize_lines_ll(){
    int i;
    _lines_ll[0].buff = (char*) malloc((LINES_LLCOUNT+1)*LINE_BUFFER_MAX_SIZE*sizeof(char));/*+2 becouse \n */
    for (i = 1; i < LINES_LLCOUNT-1; i++){
        _lines_ll[i].buff = _lines_ll[0].buff+(i*LINE_BUFFER_MAX_SIZE);
        _lines_ll[i].previous = &_lines_ll[i-1];
        _lines_ll[i].next = &_lines_ll[i+1];
        _lines_ll[i].line_number = (size_t)i;
        _lines_ll[i].buff_lenght = 0;
    }
    _lines_ll[0].next = &_lines_ll[1];
    _lines_ll[0].previous = &_lines_ll[LINES_LLCOUNT-1];
    _lines_ll[LINES_LLCOUNT-1].next = &_lines_ll[0];
    _lines_ll[LINES_LLCOUNT-1].previous = &_lines_ll[LINES_LLCOUNT-2];
    _lines_ll[LINES_LLCOUNT-1].buff = _lines_ll[0].buff+((LINES_LLCOUNT-1)*LINE_BUFFER_MAX_SIZE);
    _lines_ll[0].previous->line_number = (size_t)-1;
}

#include <sys/ioctl.h>
#include <malloc.h>
#include "positions.h"
#include "lines.h"
#include "term_out.h"
#include "more.h"

buffer_info _buff_info;
#define POSITIONS_STEP 100
#define POSITIONS_ADD_COUNT 1024

static struct winsize last_size;
static char is_size_changed;
static off_t current_offset = 0;
static size_t positions_count;
static size_t last_position;
static size_t current_line=0;
static size_t last_written_line;
static positions_s *positions;

size_t fill_positions(size_t lines_count, size_t start_count, off_t start_offset){
    struct winsize size;
    size_t count;
    skip_lines_status status;
    size_t skipped_lines_count = 0 ,i;
    size = get_term_size();
    count = lines_count/POSITIONS_STEP+1;
    last_size = size;
    if (start_count+count>positions_count) add_new_positions(count);
    for(i = start_count; i < start_count+count; i++){
        status = get_draw_bytes_count_forward(size, POSITIONS_STEP, &_buff_info, start_offset, _lines_ll, current_line);
        skipped_lines_count += status.count_lines;
        positions[i].start =start_offset;
        positions[i].end = start_offset+status.skipped_bytes_count;
        positions[i].lines_count = status.count_lines;
        positions[i].number = i*POSITIONS_STEP;
        if (status.skipped_bytes_count == 0) {
            last_position = i;
            return skipped_lines_count;
        }
        /*skip n bytes, start read from next*/
        start_offset+=status.skipped_bytes_count+1;
        _buff_info.buff_offset++;
    }
    last_position = i;
    return skipped_lines_count;
}

    void add_new_positions(size_t count){
    size_t i, pos_count;
    pos_count = ((count/POSITIONS_ADD_COUNT)+1)*POSITIONS_ADD_COUNT;
    pos_count = pos_count<POSITIONS_ADD_COUNT?POSITIONS_ADD_COUNT:pos_count;
    positions_count += pos_count;
    positions = (positions_s*)realloc(positions, sizeof(positions_s)*positions_count);
    if (positions == NULL) print_error("Can't allocate memory to buffer", 12, errno);

    for(i = positions_count-pos_count; i < positions_count; i++){
        positions[i].number = i*POSITIONS_STEP;
        positions[i].start = 0;
        positions[i].end = 0;
    }
}

/*Return offset to the n line from position*/
position_s skip(size_t start_position, size_t count_lines){
    struct winsize size;
    position_s position;
    skip_lines_status status;
    size = get_term_size();
    status = get_draw_bytes_count_forward(size, count_lines, &_buff_info, positions[start_position].start, _lines_ll, current_line);
    status.count_lines < count_lines?(position.isEnd = 1):(position.isEnd = 0);/*end of file*/
    position.offset = positions[start_position].start +status.skipped_bytes_count;
    return position;
}

/*get position of line
 *Can be applied only forward read*/
position_s get_start_offset_from_positions(size_t start_line){
    position_s position;
    struct winsize size;
    size = get_term_size();
    position.isEnd =0;
    if ((size_t)positions[last_position].number+positions[last_position].lines_count < start_line)
    {
        if (positions[last_position].lines_count < (size_t )size.ws_col*2){
            position.offset = positions[last_position-1].start;
            last_written_line = positions[last_position-1].number;
        }
        else{
            position.offset = positions[last_position].start;
            last_written_line = positions[last_position-1].number;
        }
        return position;
    }
    last_written_line = (size_t)start_line;
    /*fill_positions(start_line-last_position+1, last_position, positions[last_position].start);
    if (positions[last_position].number+positions[last_position].lines_count < start_line) position.isEnd = 1;/*End of file
    else position.isEnd = 0;
    if (position.isEnd) {
        position.offset = positions[last_position].end;
        return position;
    }
    */
    position.isEnd = 0;
    return skip(start_line/POSITIONS_STEP, start_line%POSITIONS_STEP);
}

size_t write_lines_forward(size_t count_liens){
    struct winsize size;
    skip_lines_status status;
    size_t ret_lines_count;
    size = get_term_size();
    if (size.ws_col != last_size.ws_col && size.ws_row != last_size.ws_row) is_size_changed = 1;
    status = get_draw_bytes_count_forward(size, count_liens, &_buff_info, current_offset, _lines_ll, current_line);
    /*if Readed bytes count lower than needed lines count, than write it*/
    if (status.count_lines < size.ws_row) {
        write_from_lines_ll(&_lines_ll[get_line_ll_index(-((int)status.count_lines), status.current_line)], status.count_lines);
        ret_lines_count = status.count_lines;
    }
        /*if Readed bytes count more than needed lines count, than write only last lines*/
    else {
        write_from_lines_ll(&_lines_ll[get_line_ll_index(-((int) size.ws_row), status.current_line)], size.ws_row);
        ret_lines_count = size.ws_row;
    }
    last_written_line += status.count_lines;
    current_offset += status.skipped_bytes_count;
    current_line = status.current_line+1<LINES_LLCOUNT?status.current_line+1:0;
    return ret_lines_count;
}

size_t write_lines_backward(size_t count_lines){
    struct winsize size;
    ssize_t start_position;
    size = get_term_size();
    start_position = last_written_line - count_lines - size.ws_row;
    start_position = (start_position < 0)? 0 : start_position;
    if (is_size_changed) fill_positions(last_written_line, 0, 0);
    is_size_changed = 0;
    current_offset = get_start_offset_from_positions((size_t)start_position).offset;
    if (start_position == 0) current_offset = 0;
    return write_lines_forward((size_t)size.ws_row-1);
}

void initialize_buffer_info(){
    _buff_info.buff = (char*) malloc(BUFFER_STANDART_SIZE*sizeof(char));
    if (_buff_info.buff == NULL) print_error("Can't allocate memory to buffer", 12, errno);
    _buff_info.buff_len = 0;
    _buff_info.buff_offset = 0;
}

void clear_pipe_end(){
    _buff_info.is_end = 0;
}

void clear_all_positions(){
    is_size_changed = 0;
    current_offset = 0;
    last_position = 0;
    current_line=0;
    last_written_line = 0;
    _buff_info.buff_len = 0;
    _buff_info.is_end = 0;
}

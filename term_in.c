#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include "more.h"
#include "term_in.h"
#include "positions.h"

#define SLEEP_TIME 1000

/*----------Buffer info--------*/
/* Buffer will contain more bytes, than terminal current size
 * Buffer length will be 10*1024 bytes
 * Buffer offset is current position in buffer
 * Buffer is_end is a flag that pointed to end of input data
 */

/*----------GLOBAL----------*/
static int f_descr = -1; /*Number of current file descriptor*/
static int files_count = -1;
static int current_file = -1;
static off_t current_file_offset = 0;
static char **paths;
static char is_file_regular = 0;
static char is_pipe_end = 0;
static int terminal_file_descr;
/*---------END GLOBAL-------*/

/*Read count bytes to buffer, fill buffer_info*/
ssize_t read_to_buffer_forward(buffer_info *buff_info, size_t bytes, size_t buffer_offset){
    ssize_t readed_bytes = 0;
    ssize_t all_read_bytes = 0;
    buff_info->file_start_offset = current_file_offset;
    if (is_file_regular) {
        if ((readed_bytes = pread(f_descr, buff_info->buff + buffer_offset, bytes, current_file_offset)) < 0)
            print_error("Can't read a file", 7, errno);
        if (readed_bytes < (ssize_t)bytes) buff_info->is_end = 1;
        current_file_offset += readed_bytes;
    }
    else{
        while (1) {
            readed_bytes = read(f_descr, buff_info->buff + buffer_offset, bytes-all_read_bytes);
            if (readed_bytes <= -1){
                if (errno == EAGAIN) {
                    usleep(SLEEP_TIME);
                    continue;
                }
                else print_error("Can't read a file", 7, errno);
            }
            if (readed_bytes == 0) is_pipe_end = 0;
            buff_info->is_end = 1;
            all_read_bytes += readed_bytes;
            current_file_offset += readed_bytes;
                return all_read_bytes;
        }
    }
    return readed_bytes;
}

char check_is_a_regular(char *file_path){
    struct stat stat_buff;
    if (stat(file_path, &stat_buff)<0) print_error("Can't get file stats", 13, errno);
    switch (stat_buff.st_mode & S_IFMT) {
        case S_IFIFO:
            is_file_regular = 0;
            break;
        case S_IFREG:
            is_file_regular = 1;
            break;
        default:
            print_error("Can work only with pipes or regular file", 14, errno);
    }
    return is_file_regular;
}

/*Return next file descriptor. If haven't next file return -1*/
int get_current_file_descr(){
    int local_f_descr;
    if(current_file >= files_count) return -1;
    if (strlen(paths[current_file]) == 1 &&  paths[current_file][0] == '-') {
        if (isatty(STDIN_FILENO)) print_error("Incorrect invocation: can't read from terminal",16, errno);
        f_descr = STDIN_FILENO;
        return f_descr;
    }
    if (check_is_a_regular(paths[current_file])) {
        if ((local_f_descr = open(paths[current_file], O_RDONLY)) < 0) print_error("Open", 1, errno);
    }
    else {
        if ((local_f_descr = open(paths[current_file], O_RDONLY | O_NONBLOCK)) < 0) print_error("Open", 1, errno);
    }
    f_descr = local_f_descr;
    return local_f_descr;
}

char is_a_regular(){
    return is_file_regular;
}

int init_terminal_file_descriptor(){
    if (isatty(STDERR_FILENO)) {
        if ((terminal_file_descr = dup(STDERR_FILENO)) < 0) print_error("Can't dup terminal fileno", 14, errno);
    }
    else{
        print_error("Can't detect terminal", 15, errno);
    }
    return terminal_file_descr;
}

int get_terminal_file_descriptor(){
    return terminal_file_descr;
}

int set_next_file(){
    if(close(f_descr)<0) print_error("Close file", 11, errno);
    current_file++;
    if (current_file>=files_count) return -1;
    return get_current_file_descr();
}
void set_file_offset(off_t offset){
    current_file_offset = offset;
    return;
}

char is_end_of_pipe(){
    return is_pipe_end;
}

void set_files_info(char **path_array, int local_files_count){
    paths = path_array;
    init_terminal_file_descriptor();
    files_count = local_files_count;
    current_file = 0;
    f_descr = get_current_file_descr();
}

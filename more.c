#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "term_in.h"
#include "commands.h"
#include "term_out.h"
#include "positions.h"

#define OUT_BUFFER_SIZE 2048

static char out_buff[OUT_BUFFER_SIZE];
char need_wait_when_end_of_file;

int print_error(char* message, int error_number, int err_code){
    configure_term_back();
    errno=err_code;
    perror(message);
    exit(error_number);
}

int cat_file(int input_fdescriptor, int out_fdescriptor){
    ssize_t read_ret_value, write_ret_value;
    while ((read_ret_value=read(input_fdescriptor, out_buff, OUT_BUFFER_SIZE))>=0){
            if (read_ret_value == 0) return 0;
            if ((write_ret_value = write(out_fdescriptor, out_buff, (size_t)read_ret_value)) >= 0){
                if (read_ret_value > write_ret_value)
                     print_error("Invalid output bytes count", 3, EIO);
                continue;
            }
            print_error("Write cat", 0, errno);
        }
    print_error("Read", 2, errno);
    return 0;
}

void cat_files(char* paths[], int count){
    int i, f_descriptor;
    if (count <= 0) return;
    for (i=0; i<count; i++){
        if (paths[i][0] == '-' && paths[i][1] == '\0') f_descriptor = 1;
        else {
            if((f_descriptor = open(paths[i], O_RDONLY)) < 0) print_error(paths[i], 1, errno);
        }
        if ((write(STDOUT_FILENO, paths[i], strlen(paths[i]))) < 0 ||
            write(STDOUT_FILENO, " FILE\n", 6)<0) print_error("Write main", 4, errno);
        cat_file(f_descriptor, STDOUT_FILENO);
        close(f_descriptor);
   }
}

int main(int argc, char* argv[]){
    int c;
    if(argc <= 1) {
        argv[1] = "-\0";
        argc = 2;
    }
    while ((c = getopt(argc,argv,"n:w")) != -1){
        switch (c){
            case 'n':
                set_term_row_count(optarg);
                break;
            case 'w':
                need_wait_when_end_of_file = 1;
                break;
            case '?':
                print_error("Invalid arguments: use -n maxlines and +startline", 18, errno);
                break;
        };
    };
    argv += optind;
    argc -= optind;
    if(!isatty(STDOUT_FILENO)) {
        cat_files(argv, argc);
        exit(0);
    }
    set_files_info(argv, argc);
    initialize_buffer_info();
    initialize_lines_ll();
    command(argv, ' ');
    return 0; 
} 

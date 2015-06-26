#include "commands.h"
#include <sys/ioctl.h>
#include <termio.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include "more.h"
#include "term_out.h"
#include "positions.h"

#define LOCAL_BUFFER_LENGTH 128*4

static char local_buff[LOCAL_BUFFER_LENGTH];
static size_t repeat_command = 0;
static int current_file;
static struct termios old, new;
char was_term_configured;
extern char need_wait_when_end_of_file;

void configure_term_back(){
    if (was_term_configured) if(ioctl(get_terminal_file_descriptor(), TCSETS, &old)<0) print_error("Configure terminal set", 8, errno);
}

void kill_last_line(){
    int i = 0;
    struct winsize size;
    size = get_term_size();
    for(i = 0; i < LOCAL_BUFFER_LENGTH; i++) local_buff[i] = ' ';
    write_out("\r",1);
    write_out(local_buff, (LOCAL_BUFFER_LENGTH<(ssize_t)size.ws_col?LOCAL_BUFFER_LENGTH:(size_t)size.ws_col));
    write_out("\r",1);
}

void configure_term(){
    if(ioctl(get_terminal_file_descriptor(), TCGETS, &old)<0) print_error("Configure terminal get", 8, errno);
    new = old;
    new.c_lflag &= ~(ICANON|ECHO);
    new.c_cc[VMIN] = 1;
    new.c_cc[VTIME] = 0;
    if(ioctl(get_terminal_file_descriptor(), TCSETS, &new)<0) print_error("Configure terminal set", 8, errno);
    was_term_configured = 1;
}

char get_char(){
    char c;
    if (read(get_terminal_file_descriptor(), &c, 1) <= 0) print_error("Read char from terminal", 9, errno);
    return c;
}

void write_last_line(char *last_line, struct winsize *term_size){
    write_out("---", 3);
    if (strlen(last_line) == 1 &&  last_line[0] == '-' ) last_line = "STDIN";
    write_out(last_line, ((ssize_t)strlen(last_line))-6<(ssize_t)term_size->ws_col?strlen(last_line):(size_t)(term_size->ws_col-6));
    write_out("---", 3);
}

void command(char **file_names, char start_char) {
    char ch;
    char *last_line;
    struct winsize size;
    char eof=0, is_start_char=1, is_regular_file = 0, write_new_line = 1;
    configure_term();
    size = get_term_size();
    while (1) {
        repeat_command = 0;
        if (start_char) is_regular_file = is_a_regular();
        while (1) {
            last_line = file_names[current_file];
            if (!is_start_char) ch = get_char();
            else{
                ch = start_char;
                if (!is_regular_file) {
                    size.ws_row--; /*because last line is name of file*/
                    while (size.ws_row--)
                        write_lines_forward(1);
                        if (is_end_of_pipe()) {
                            eof = 1;
                            write_last_line(last_line, &size);
                            break;
                        }
                    write_last_line(last_line, &size);
                    break;
                }
            }
            if (isdigit(ch)) {
                repeat_command = (repeat_command * 10 + ch - '0')<repeat_command?repeat_command:(repeat_command * 10 + ch - '0');
                continue;
            }
            size = get_term_size();
            if (repeat_command == 0) repeat_command++;
            if(is_start_char) is_start_char = 0;
            else kill_last_line();
            if (eof){
                if (ch != 'b' ) {
                    if (need_wait_when_end_of_file && ch!='\n' && ch!='q') continue;
                    if (set_next_file() < 0) ch = 'q';
                    else {
                        current_file++;
                        is_regular_file = is_a_regular();
                        clear_all_positions();
                        eof = 0;
                    }
                }
                else eof = 0;
            }
            if(ch =='b' && !is_regular_file) {
                last_line = "can't go back";
                write_last_line(last_line, &size);
                break;
            }
            switch (ch) {
                case ' ':
                    if(write_lines_forward(repeat_command*size.ws_row-1) <= 0) eof = 1;
                    break;
                case 'b':
                    if(write_lines_backward(repeat_command*size.ws_row-1) <= 0) eof = 1;
                    break;
                case '\n': /*return symbol*/
                    if(write_lines_forward(repeat_command) <=0) eof = 1;
                    break;
                case 'q':
                    configure_term_back();
                    exit(0);
                default:
                    last_line = "Error command";
                    write_new_line = 0;
                    break;
            }
            if (eof) last_line = "End";
            if (!is_regular_file) {
                if(is_end_of_pipe()) eof = 1;
                else clear_pipe_end();
            }
            repeat_command = 0;
            if (!is_last_writed_symbol_the_new_line() && write_new_line) write_out("\n", 1);
            write_last_line(last_line, &size);
        }
    }
}

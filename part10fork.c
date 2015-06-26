#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "part10fork.h"
#include "overal_functions.h"

#define BUFF_SIZE 4096
#define WC_PATH "/usr/bin/wc"

static char buff[BUFF_SIZE];
static char even_buff[BUFF_SIZE/2];

int main(int argc, char **argv){
    int filedes[2];
    int fdescr;
    ssize_t readed_bytes_count, i;

    if (argc<2)
        print_error("Invalid arguments count", 1, errno);
    if ((fdescr = open(argv[1], O_RDONLY)) <0)
        print_error("Can't open a file", 2, errno);
    if (pipe(filedes)!=0)
        print_error("Can't create a pipe", 3, errno);
    switch (fork()) {
        case -1:
            print_error("Can't fork", 4, errno);
        case 0:
            if (close(filedes[1])<0 || close(fdescr)<0)
                print_error("Can't close file descriptor", 7, errno);
            if (dup2(filedes[0], STDIN_FILENO) < 0)
                print_error("Can't dup file descriptor", 8, errno);
            if (close(filedes[0])<0)
                print_error("Can't close file descriptor", 9, errno);
            if (execl(WC_PATH, WC_PATH, NULL)<0)
                print_error("Can't exec wc", 10, errno);
        default:
            if(close(filedes[0])<0)
                print_error("Can't close a file", 7, errno);
            while ((readed_bytes_count = read(fdescr, &buff, BUFF_SIZE)))
            {
                if (readed_bytes_count<0)
                    print_error("Can't read file", 5, errno);
                for (i = 0; i < readed_bytes_count; i++)
                    even_buff[i>>1] = buff[i];
                if (write(filedes[1], &even_buff, i>>1)<0)
                    print_error("Can't write to pipe", 6, errno);
            }
            if (close(filedes[1])<0 || close(fdescr)< 0)
                print_error("Can't close a file", 7, errno);
    }
    return EXIT_SUCCESS;
}

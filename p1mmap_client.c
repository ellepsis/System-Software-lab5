#include <signal.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "p1mmap_client.h"
#include "part1serv.h"
#include "overal_functions.h"

static char* mmap_addres;
int fd = -1;
struct serv_info{
    time_t running_time;
    process_info proc_info;
    double load_avg[3];
}info;

void sigterm_handler(int signal){
    if (fd >=0) close(fd);
    exit(EXIT_SUCCESS);
}

void connect_to_mmap(){
    if ((fd = open(PATH, O_RDONLY)) < 0)
        print_error("Can't open file", 11, errno);
    if ((mmap_addres = (char*)
            mmap(NULL, INFO_SIZE, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED)
        print_error("Cant' mmap file", 12, errno);
}

int main(){
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    print_start_message();
    connect_to_mmap();
    while (1){
        memcpy(&info, mmap_addres, sizeof(info));
        printf("Server uptime: %lld\n", (long long)info.running_time);
        printf("Server pid: %d\n", info.proc_info.proc_id);
        printf("Server uid: %d\n", info.proc_info.user_id);
        printf("Server gid: %d\n", info.proc_info.group_id);
        printf("System average load on last 1 minute: %f\n", info.load_avg[0]);
        printf("System average load on last 5 minute: %f\n", info.load_avg[1]);
        printf("System average load on last 15 minute: %f\n", info.load_avg[2]);
        sleep(1);
    }
    return EXIT_SUCCESS;
}

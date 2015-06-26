#include "part8_serv_uds.h"
/*
// Created by ellepsis on 16.05.15.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/loadavg.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/un.h> //SOLARIS MUST BE REMOVED
#include <sys/socket.h>
#include <sys/select.h>
#include "part1serv.h"
#include "overal_functions.h"

#define SLEEPTIME 1
#define MAX_WAIT_TIME 20

static char* shm_addres;

struct serv_info{
    time_t running_time;
    process_info proc_info;
    double load_avg[3];
}info;


void save_process_info(){
    if ((info.proc_info.proc_id = getpid())< 0)
        print_error("Can't get process id", 1, errno);
    if ((info.proc_info.user_id = getuid())< 0)
        print_error("Can't get user id", 2, errno);
    if ((info.proc_info.group_id = getgid())< 0)
        print_error("Can't get group id", 3, errno);
}

void sigterm_handler(int signal){
    if (shm_addres != NULL && shm_addres != (char*)-1)
    if (shmdt(shm_addres)<0) print_error("Can't release shared memory", 8,errno);
    exit(EXIT_SUCCESS);
}

int main(){
    int socket_fd, socket_rcv_fd, socket_flags;
    time_t start_time;
    struct sockaddr_un socket_path;
    fd_set socket_set;
    struct timeval timeout;

    if ((start_time = time(NULL))<=0) print_error("Can't get time", 4, errno);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    print_start_message();
    save_process_info();

    /*Initialize socket*/
    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0))<0)
        print_error("Can't create a socket", 20, errno);
    socket_path.sun_family = AF_UNIX;
    strcpy(socket_path.sun_path, SOCKET_PATH);
    unlink(SOCKET_PATH);
    if (bind(socket_fd, (struct sockaddr*)(&socket_path),
             sizeof(socket_path))!=0)
        print_error("Cat't bind the socket", 21, errno);
    if (listen(socket_fd, 1)!=0)
        print_error("Can't set socket listen", 22, errno);
    if ((socket_flags = fcntl(socket_fd, F_GETFL, 0))< 0)
        print_error("Can't get socket flags", 23, errno);
    if (fcntl(socket_fd, F_SETFL, socket_flags|O_NONBLOCK)<0)
        print_error("Can't set socket unblock flag", 24, errno);
    timeout.tv_sec = MAX_WAIT_TIME;
    /*Initialize end*/

    while (1){
        FD_CLR(socket_fd, &socket_set);
        FD_SET(socket_fd, &socket_set);
        if ((info.running_time = time(NULL))<=0)
            print_error("Can't get time", 4, errno);
        info.running_time = info.running_time-start_time;
        if (getloadavg(info.load_avg, LOADSAMPLESCOUNT)<LOADSAMPLESCOUNT)
            print_error("Can't get load info", 5, errno);
        if (select(socket_fd+1, &socket_set, NULL, NULL, &timeout)<0)
            print_error("Error socket select()", 25, errno);
        if (FD_ISSET(socket_fd, &socket_set)){
            if ((socket_rcv_fd = accept(socket_fd, NULL, NULL))<0)
                print_error("Can't accept message", 26, errno);
            if (send(socket_rcv_fd, &info, sizeof(info),0)<0)
                print_error("Can't send a message", 27, errno);
            if (close(socket_rcv_fd)<0)
                print_error("Can't close socket fd", 28, errno);
        }
    }
    if (shmdt(shm_addres)<0) print_error("Can't realise shared memory"
                                                 "reload machine", 8,errno);
    return EXIT_SUCCESS;
}

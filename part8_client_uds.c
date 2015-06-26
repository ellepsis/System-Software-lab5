#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <sys/un.h>
#include "part1serv.h"
#include "part8_serv_uds.h"
#include "overal_functions.h"

struct serv_info{
    time_t running_time;
    process_info proc_info;
    double load_avg[3];
}info;

void sigterm_handler(int signal){
    exit(EXIT_SUCCESS);
}

int main(){
    int socket_fd;
    struct sockaddr_un socket_path;

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    strcpy(socket_path.sun_path, SOCKET_PATH);
    socket_path.sun_family = AF_UNIX;
    print_start_message();

    while (1){
        if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0))< 0)
            print_error("Can't create a socket", 20, errno);
        if (connect(socket_fd, (struct sockaddr*)(&socket_path), sizeof
        (socket_path))<0)
            print_error("Can't connect to server", 21, errno);
        if (recv(socket_fd, &info, sizeof(info), 0) < 0)
            print_error("Can't recive a message", 22, errno);
        close(socket_fd);
        printf("Server uptime: %lld\n", (long long)info.running_time);
        printf("Server pid: %d\n", info.proc_info.proc_id);
        printf("Server uid: %d\n", info.proc_info.user_id);
        printf("Server gid: %d\n", info.proc_info.group_id);
        printf("System average load on last 1 minute: %f\n",info.load_avg[0]);
        printf("System average load on last 5 minute: %f\n",info.load_avg[1]);
        printf("System average load on last 15 minute: %f\n",info.load_avg[2]);
        sleep(1);
    }
    return EXIT_SUCCESS;
}

#include "p1msgq_client.h"
#include "overal_functions.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include "part1serv.h"
#include "overal_functions.h"


int msgq_id;

struct serv_info{
    time_t running_time;
    process_info proc_info;
    double load_avg[3];
}info;

struct my_message{
    long mtype;
    struct serv_info s_info;
}mymsg;

void sigterm_handler(int signal){
    exit(EXIT_SUCCESS);
}

void connect_to_msgq(){
    key_t key;
    if ((key = ftok(KEY_PATH, KEY_ID))<0)
        print_error("Can't create key", 5, errno);
    if ((msgq_id = msgget(key, IPC_EXCL))< 0)
        print_error("Can't open shared memory. Please start server", 6, errno);
}

void receive_message(){
    static size_t retry_count;
    if (msgrcv(msgq_id, &mymsg, sizeof(mymsg)-sizeof(long), MSG_TYPE, 0) < 0){
        if (errno == EINTR){
            if (retry_count++<3){
                receive_message();
                retry_count = 0;
            }
            else
            {
                print_error("Can't recieve message: Interupt", 7, errno);
            }
        }
        else{
            print_error("Can't recieve message", 8, errno);
        }
    }
}

int main(){
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    print_start_message();
    connect_to_msgq();
    while (1){
        receive_message();
        printf("Server uptime: %lld\n", (long long)mymsg.s_info.running_time);
        printf("Server pid: %d\n", mymsg.s_info.proc_info.proc_id);
        printf("Server uid: %d\n", mymsg.s_info.proc_info.user_id);
        printf("Server gid: %d\n", mymsg.s_info.proc_info.group_id);
        printf("System average load on last 1 minute: %f\n",mymsg.s_info.load_avg[0]);
        printf("System average load on last 5 minute: %f\n",mymsg.s_info.load_avg[1]);
        printf("System average load on last 15 minute: %f\n",mymsg.s_info.load_avg[2]);
        sleep(1);
    }
    return EXIT_SUCCESS;
}

#include <signal.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "p1shm_client.h"
#include "part1serv.h"
#include "overal_functions.h"

static char* shm_addres;
struct serv_info{
    time_t running_time;
    process_info proc_info;
    double load_avg[3];
}info;

void sigterm_handler(int signal){
    if (shm_addres != NULL && shm_addres != (char*)-1)
    if (shmdt(shm_addres)<0)
        print_error("Can't release shared memory", 8,errno);
    exit(EXIT_SUCCESS);
}

void connect_to_shm(){
    key_t key;
    int shm_id;
    if ((key = ftok(KEY_PATH, KEY_ID))<0)
        print_error("Can't create key", 5, errno);
    if ((shm_id = shmget(key, INFO_SIZE, IPC_EXCL))< 0)
        print_error("Can't open shared memory. Please start server", 6, errno);
    if ((shm_addres = shmat(shm_id, NULL, 0)) == (char *)(-1))
        print_error("Can't attach shared memory", 7, errno);
}

int main(){
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    print_start_message();
    connect_to_shm();
    while (1){
        memcpy(&info, shm_addres, sizeof(info));//bug
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

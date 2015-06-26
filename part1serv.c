/*
// Created by ellepsis on 16.05.15.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/loadavg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "part1serv.h"
#include "overal_functions.h"

#define SLEEPTIME 1

static char* shm_addres, *mmap_addres;
static char* tmp_buff[INFO_SIZE+sizeof(long)]; /*rcv buff*/
static int msgq_id;
static int fd = -1;

struct serv_info{
    time_t running_time;
    process_info proc_info;
    double load_avg[3];
}info;

struct {
    long mtype;
    struct serv_info s_info;
}mymsg;

void save_process_info(){
    if ((info.proc_info.proc_id = getpid())< 0)
        print_error("Can't get process id", 1, errno);
    if ((info.proc_info.user_id = getuid())< 0)
        print_error("Can't get user id", 2, errno);
    if ((info.proc_info.group_id = getgid())< 0)
        print_error("Can't get group id", 3, errno);
}

void create_shm(){
    key_t key;
    int shm_id;
    if ((key = ftok(KEY_PATH, KEY_ID))<0)
        print_error("Can't create key", 5, errno);
    if ((shm_id = shmget(key, INFO_SIZE, PERMS | IPC_CREAT))< 0)
        print_error("Can't create shared memory", 6, errno);
    if ((shm_addres = (char*) shmat(shm_id, NULL, 0))==(char*)-1)
        print_error("Can't attach shared memory", 7, errno);
}

void send_message(){
    mymsg.mtype = MSG_TYPE;
    mymsg.s_info = info;
    if(msgrcv(msgq_id, &tmp_buff, sizeof(mymsg)-sizeof(long), MSG_TYPE, IPC_NOWAIT)<0)
        if (errno != ENOMSG) print_error("Can't clear mssages", 1,errno);
    if(msgsnd(msgq_id, &mymsg, sizeof(mymsg)-sizeof(long), 0)<0)
        print_error("Can't sent message", 1, errno);
}

void create_msg_queue(){
    key_t key;
    if ((key = ftok(KEY_PATH, KEY_ID))<0)
        print_error("Can't create key", 5, errno);
    if ((msgq_id = msgget(key, PERMS|IPC_CREAT))< 0)
        print_error("Can't create shared memory", 6, errno);
}

void create_mmap(){
    if ((fd = open(PATH, O_RDWR| O_CREAT, PERMS))<0)
        print_error("Can't open file", 9, errno);
    if (ftruncate(fd, INFO_SIZE) < 0)
        print_error("Can't truncate file", 10, errno);
    if ((mmap_addres = mmap(0, INFO_SIZE, PROT_WRITE, MAP_SHARED, fd, 0)) ==
            (char *) -1)
        print_error("Can't mmap file", 11, errno);
}

void sigterm_handler(int signal){
    if (shm_addres != NULL && shm_addres != (char*)-1)
        if (shmdt(shm_addres)<0) print_error("Can't release shared memory", 8,errno);
    if (fd>=0) close(fd);
    exit(EXIT_SUCCESS);
}

int main(){
    time_t start_time;
    if ((start_time = time(NULL))<=0) print_error("Can't get time", 4, errno);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    print_start_message();
    save_process_info();
    create_shm();
    create_msg_queue();
    create_mmap();
    while (1){
        if ((info.running_time = time(NULL))<=0)
            print_error("Can't get time", 4, errno);
        info.running_time = info.running_time-start_time;
        if (getloadavg(info.load_avg, LOADSAMPLESCOUNT)<LOADSAMPLESCOUNT)
            print_error("Can't get load info", 5, errno);
        memcpy(shm_addres, &info, sizeof(info));
        memcpy(mmap_addres, &info, sizeof(info));
        send_message();
        sleep(SLEEPTIME);
    }
    if (shmdt(shm_addres)<0) print_error("Can't realise shared memory"
                                                 "reload machine", 8,errno);
    return EXIT_SUCCESS;
}

#ifndef LAB5_PART1_H
#define LAB5_PART1_H

#include <unistd.h>

#define LOADSAMPLESCOUNT 3
#define KEY_PATH "/home/s182685/workdir/c/lab5/"
#define KEY_ID 100
#define INFO_SIZE sizeof(double)*LOADSAMPLESCOUNT+sizeof(process_info)+ sizeof(time_t)
#define MSG_TYPE 4
#define PATH "/tmp/tmpfile123"
#define PERMS 0644


typedef struct process_info{
    pid_t proc_id;
    uid_t user_id;
    gid_t group_id;
} process_info;

#endif

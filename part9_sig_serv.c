#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/loadavg.h>
#include "part1serv.h"
#include "overal_functions.h"

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

void signals_handler(int sig)
{
    switch(sig)
    {
        case SIGHUP:
            printf("Server uptime: %lld\n", (long long)info.running_time);
            break;
        case SIGINT:
            printf("Server pid: %d\n", info.proc_info.proc_id);
            break;
        case SIGTERM:
            printf("Server uid: %d\n", info.proc_info.user_id);
            break;
        case SIGUSR1:
            printf("Server gid: %d\n", info.proc_info.group_id);
            break;
        case SIGUSR2:
            printf("System average load on last 1 minute: %f\n",info.load_avg[0]);
            printf("System average load on last 5 minute: %f\n",info.load_avg[1]);
            printf("System average load on last 15 minute: %f\n",info.load_avg[2]);
            break;
        default:
            printf("%s\n", "Invalid signal");
            break;
    }
}

int main() {
    sigset_t set;
    time_t start_time;
    struct sigaction action;

    start_time = time(NULL);
    save_process_info();
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);

    action.sa_flags = 0;
    action.sa_mask = set;
    action.sa_handler = signals_handler;

    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

    while (1) {
        if ((info.running_time = time(NULL)) <= 0)
            print_error("Can't get time", 4, errno);
        info.running_time = info.running_time - start_time;
        if (getloadavg(info.load_avg, LOADSAMPLESCOUNT) < LOADSAMPLESCOUNT)
            print_error("Can't get load info", 5, errno);
        sleep(1);
    }
    return 1;
}

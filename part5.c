#include <stddef.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/sem.h>
#include "overal_functions.h"
#include "part1serv.h"


#define SYMBOLS_COUNT 26
#define SEMAPHORES_COUNT 3
#define SLEEP_TIME 1
#define SEM_ARRAY 0
#define SEM_FOO_END 1
#define SEM_REGISTER 2

char symbols[SYMBOLS_COUNT+1];

int sem_id;
pthread_t reg_thr, arr_inv_thr;

void *invert_register(){
    size_t i;
    int to_upper = 0;
    struct sembuf sem_start, sem_end;

    sem_start.sem_num = SEM_REGISTER;
    sem_end.sem_num = SEM_FOO_END;
    sem_start.sem_flg = SEM_UNDO;
    sem_end.sem_flg = SEM_UNDO;
    sem_start.sem_op = -1;
    sem_end.sem_op = 1;

    while (1) {
        if(semop(sem_id, &sem_start, 1)<0)
            print_error("Can't set a semaphore", 7, errno);
        to_upper = islower((int) symbols[0]);
        for (i = 0; i < SYMBOLS_COUNT; i++) {
            symbols[i] = to_upper ?
                         (char) toupper((int) symbols[i]) :
                         (char) tolower((int) symbols[i]);
        }
        if(semop(sem_id, &sem_end, 1)<0)
            print_error("Can't set a semaphore", 7, errno);
    }
}

void *invert_array(){
    size_t i;
    char tmpelem;
    struct sembuf sem_start, sem_end;

    sem_start.sem_num = SEM_ARRAY;
    sem_end.sem_num = SEM_FOO_END;
    sem_start.sem_flg = SEM_UNDO;
    sem_end.sem_flg = SEM_UNDO;
    sem_start.sem_op = -1;
    sem_end.sem_op = 1;

    while (1) {
        if(semop(sem_id, &sem_start, 1)<0)
            print_error("Can't set a semaphore", 7, errno);
        for (i = 0; i < SYMBOLS_COUNT / 2; i++) {
            tmpelem = symbols[i];
            symbols[i] = symbols[SYMBOLS_COUNT - i - 1];
            symbols[SYMBOLS_COUNT - i - 1] = tmpelem;
        }
        if(semop(sem_id, &sem_end, 1)<0)
            print_error("Can't set a semaphore", 7, errno);
    }
}

void create_semaphores(){
    key_t key;
    if ((key = ftok(KEY_PATH, KEY_ID+23))<0)
        print_error("Can't create key", 5, errno);
    if ((sem_id = semget(key, SEMAPHORES_COUNT, PERMS | IPC_CREAT)) < 0)
        print_error("Can't get semaphores", 6, errno);
}

void sigterm_handler(int signal){
    if (pthread_cancel(reg_thr) != 0)
        print_error("Can't cancel the thread",3, errno);
    if (pthread_cancel(arr_inv_thr) != 0)
        print_error("Can't cancel the thread",3, errno);
    exit(EXIT_SUCCESS);
}

int main(){
    size_t i;
    struct sembuf sem_start_array, sem_start_register, sem_wait_foo_end;

    for (i = 0; i<SYMBOLS_COUNT; i++){
        symbols[i] = (char)i+'a';
    }
    symbols[SYMBOLS_COUNT] = '\0';

    create_semaphores();
    sem_wait_foo_end.sem_num = SEM_FOO_END;
    sem_start_array.sem_num = SEM_ARRAY;
    sem_start_register.sem_num = SEM_REGISTER;
    sem_wait_foo_end.sem_flg = SEM_UNDO;
    sem_start_array.sem_flg = SEM_UNDO;
    sem_start_register.sem_flg = SEM_UNDO;
    sem_wait_foo_end.sem_op = -1;
    sem_start_array.sem_op = 1;
    sem_start_register.sem_op = 1;

    if (pthread_create(&reg_thr, NULL, &invert_register, NULL) != 0)
        print_error("Can't create a thread", 2, errno);
    if (pthread_create(&arr_inv_thr, NULL, &invert_array, NULL) != 0)
        print_error("Can't create a thread", 2, errno);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    while (1){
        if(semop(sem_id, &sem_start_array, 1)<0)
            print_error("Can't set a semaphore", 7, errno);
        if(semop(sem_id, &sem_wait_foo_end, 1)<0)
            print_error("Can't set a semaphore", 7, errno);
        sleep(SLEEP_TIME);
        printf("%s\n", symbols);
        if(semop(sem_id, &sem_start_register, 1)<0)
            print_error("Can't set a semaphore", 7, errno);
        if(semop(sem_id, &sem_wait_foo_end, 1)<0)
            print_error("Can't set a semaphore", 7, errno);
        sleep(SLEEP_TIME);
        printf("%s\n", symbols);
    }
}





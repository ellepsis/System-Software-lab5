#include <stddef.h>
#include <ctype.h>
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "overal_functions.h"
#include "part4.h"

#define SYMBOLS_COUNT 26
#define SLEEP_TIME 1

char symbols[SYMBOLS_COUNT+1];

sem_t sem_register, sem_array, sem_foo_end;
pthread_t reg_thr, arr_inv_thr;

void *invert_register(){
    size_t i;
    int to_upper = 0;
    while (1) {
        if(sem_wait(&sem_register)<0)
            print_error("Can't set semaphore", 9, errno);
        to_upper = islower((int) symbols[0]);
        for (i = 0; i < SYMBOLS_COUNT; i++) {
            symbols[i] = to_upper ?
                         (char) toupper((int) symbols[i]) :
                         (char) tolower((int) symbols[i]);
        }
        if(sem_post(&sem_foo_end)<0)
            print_error("Can't set semaphore", 9, errno);
    }
}

void *invert_array(){
    size_t i;
    char tmpelem;
    while (1) {
        if(sem_wait(&sem_array)<0)
            print_error("Can't set semaphore", 9, errno);
        for (i = 0; i < SYMBOLS_COUNT / 2; i++) {
            tmpelem = symbols[i];
            symbols[i] = symbols[SYMBOLS_COUNT - i - 1];
            symbols[SYMBOLS_COUNT - i - 1] = tmpelem;
        }
        if(sem_post(&sem_foo_end)<0)
            print_error("Can't set semaphore", 9, errno);
    }
}

void sigterm_handler(int signal){
    if (pthread_cancel(reg_thr) != 0)
        print_error("Can't cancel the thread",3, errno);
    if (pthread_cancel(arr_inv_thr) != 0)
        print_error("Can't cancel the thread",3, errno);
    if (sem_destroy(&sem_array)<0)
        print_error("Can't destory the semaphore", 4, errno);
    if (sem_destroy(&sem_register)<0)
        print_error("Can't destory the semaphore", 4, errno);
    if (sem_destroy(&sem_foo_end)<0)
        print_error("Can't destory the semaphore", 4, errno);
    exit(EXIT_SUCCESS);
}

int main(){
    size_t i;
    for (i = 0; i<SYMBOLS_COUNT; i++){
        symbols[i] = (char)i+'a';
    }
    symbols[SYMBOLS_COUNT] = '\0';
    if (sem_init(&sem_register, 0, 0) < 0 || sem_init(&sem_array, 0, 0) <0 ||
        sem_init(&sem_foo_end, 0, 0) <0)
        print_error("Can't create a semaphore", 1, errno);
    if (pthread_create(&reg_thr, NULL, &invert_register, NULL) != 0)
        print_error("Can't create a thread", 2, errno);
    if (pthread_create(&arr_inv_thr, NULL, &invert_array, NULL) != 0)
        print_error("Can't create a thread", 2, errno);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    while (1){
        if(sem_post(&sem_array)<0)
            print_error("Can't set semaphore", 9, errno);
        if(sem_wait(&sem_foo_end)<0)
            print_error("Can't set semaphore", 9, errno);
        sleep(SLEEP_TIME);
        printf("%s\n", symbols);
        if(sem_post(&sem_register)<0)
            print_error("Can't set semaphore", 9, errno);
        if(sem_wait(&sem_foo_end)<0)
            print_error("Can't set semaphore", 9, errno);
        sleep(SLEEP_TIME);
        printf("%s\n", symbols);
    }
}





#include "part6.h"
#include <stddef.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include "overal_functions.h"

#define SYMBOLS_COUNT 26
#define SLEEP_TIME 1000000

char symbols[SYMBOLS_COUNT+1];

pthread_mutex_t mutex_register, mutex_array, mutex_foo_end;
pthread_t reg_thr, arr_inv_thr;

void *invert_register(){
    size_t i;
    int to_upper = 0;
    while (1) {
        if (pthread_mutex_lock(&mutex_register)<0)
            print_error("Can't set mutex", 9, errno);
        to_upper = islower((int) symbols[0]);
        for (i = 0; i < SYMBOLS_COUNT; i++) {
            symbols[i] = to_upper ?
                         (char) toupper((int) symbols[i]) :
                         (char) tolower((int) symbols[i]);
        }
        if (pthread_mutex_unlock(&mutex_foo_end)<0)
            print_error("Can't set mutex", 9, errno);
    }
}

void *invert_array(){
    size_t i;
    char tmpelem;
    while (1) {
        if (pthread_mutex_lock(&mutex_array)<0)
            print_error("Can't set mutex", 9, errno);
        for (i = 0; i < SYMBOLS_COUNT / 2; i++) {
            tmpelem = symbols[i];
            symbols[i] = symbols[SYMBOLS_COUNT - i - 1];
            symbols[SYMBOLS_COUNT - i - 1] = tmpelem;
        }
        if (pthread_mutex_unlock(&mutex_foo_end)<0)
            print_error("Can't set mutex", 9, errno);
    }
}

void sigterm_handler(int signal){
    if (pthread_cancel(reg_thr) != 0)
        print_error("Can't cancel the thread",3, errno);
    if (pthread_cancel(arr_inv_thr) != 0)
        print_error("Can't cancel the thread",3, errno);
    if (pthread_mutex_destroy(&mutex_array)<0)
        print_error("Can't destory the mutex", 4, errno);
    if (pthread_mutex_destroy(&mutex_foo_end)<0)
        print_error("Can't destory the mutex", 4, errno);
    if (pthread_mutex_destroy(&mutex_register)<0)
        print_error("Can't destory the mutex", 4, errno);
    exit(EXIT_SUCCESS);
}

int main(){
    size_t i;
    for (i = 0; i<SYMBOLS_COUNT; i++){
        symbols[i] = (char)i+'a';
    }
    symbols[SYMBOLS_COUNT] = '\0';
    if (pthread_mutex_init(&mutex_array,NULL)<0 ||
            pthread_mutex_init(&mutex_foo_end,NULL)<0 ||
            pthread_mutex_init(&mutex_register,NULL)<0)
        print_error("Can't create a semaphore", 1, errno);
    if (pthread_create(&reg_thr, NULL, &invert_register, NULL) != 0)
        print_error("Can't create a thread", 2, errno);
    if (pthread_create(&arr_inv_thr, NULL, &invert_array, NULL) != 0)
        print_error("Can't create a thread", 2, errno);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    while (1){
        if (pthread_mutex_unlock(&mutex_array)<0)
            print_error("Can't set mutex", 9, errno);
        if (pthread_mutex_lock(&mutex_foo_end)<0)
            print_error("Can't set mutex", 9, errno);
        printf("%s\n", symbols);
        usleep(SLEEP_TIME);
        if (pthread_mutex_unlock(&mutex_register)<0)
            print_error("Can't set mutex", 9, errno);
        if (pthread_mutex_lock(&mutex_foo_end)<0)
            print_error("Can't set mutex", 9, errno);
        printf("%s\n", symbols);
        usleep(SLEEP_TIME);
    }
}





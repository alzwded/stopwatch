#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#ifndef SLEEP_AMOUNT
#define SLEEP_AMOUNT 10
#endif

struct timespec t;
pid_t child;

void gettime(struct timespec* t)
{
    clock_gettime(CLOCK_REALTIME, t);
}

struct timespec diff(struct timespec* start, struct timespec* end)
{
    struct timespec d;
    if((end->tv_nsec - start->tv_nsec) < 0) {
        d.tv_sec = end->tv_sec - start->tv_sec - 1;
        d.tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
    } else {
        d.tv_sec = end->tv_sec - start->tv_sec;
        d.tv_nsec = end->tv_nsec - start->tv_nsec;
    }
    return d;
}

void printtime(struct timespec* d)
{
    time_t seconds, minutes, hours;
    long centis;
    seconds = d->tv_sec % 60;
    minutes = d->tv_sec / 60 % 60;
    hours = d->tv_sec / 3600;
    centis = d->tv_nsec / 10000000;
    printf("%ld:%02ld:%02ld.%ld\n", hours, minutes, seconds, centis);
}

void handle(int signum)
{
    switch(signum)
    {
    case SIGINT: {
        if(child) {
            struct timespec now, d;

            gettime(&now);
            d = diff(&t, &now);

            printf("\ntotal:   ");
            printtime(&d);
            fflush(stdout);

            gettime(&t);
        } else {
            gettime(&t);
        }
        break; }
    case SIGQUIT: {
        if(child) kill(child, SIGQUIT);
        exit(0);
        break; }
    };
}

void printTimeEveryNowAndThen()
{
    struct timespec now, d;
    sleep(SLEEP_AMOUNT);
    gettime(&now);
    d = diff(&t, &now);
    // XXX whatever
    if(!(d.tv_nsec / 10000000 == 0 && d.tv_sec == 0)) {
        printf("elapsed: ");
        printtime(&d);
        fflush(stdout);
    }
}

void reap(int signum)
{
    int status;
    waitpid(child, &status, WNOHANG);
}

int main(int argc, char* argv[])
{
    gettime(&t);
    sigset_t mask;

    signal(SIGINT, handle);
    signal(SIGQUIT, handle);
    signal(SIGCHLD, reap);
    sigemptyset(&mask);
    sigdelset(&mask, SIGINT);
    sigdelset(&mask, SIGQUIT);
    sigdelset(&mask, SIGHUP);
    sigdelset(&mask, SIGCONT);
    sigdelset(&mask, SIGSTOP);
    sigdelset(&mask, SIGTSTP);
    sigdelset(&mask, SIGCHLD);
    if(!(child = fork()))
    {
        signal(SIGINT, handle);
        signal(SIGQUIT, handle);
        do {printTimeEveryNowAndThen();} while(1);
    } else {
        do {} while(sigsuspend(&mask));
    }
    return 0;
}

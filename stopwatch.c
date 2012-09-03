/*
Copyright (c) 2012, Vlad Mesco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
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

void printtime(FILE* f, const char* msg, struct timespec* d)
{
    time_t seconds, minutes, hours;
    long centis;
    seconds = d->tv_sec % 60;
    minutes = d->tv_sec / 60 % 60;
    hours = d->tv_sec / 3600;
    centis = d->tv_nsec / 10000000;
    fprintf(f, "%s%ld:%02ld:%02ld.%02ld\n", msg, hours, minutes, seconds, centis);
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

            printtime(stdout, "total:   ", &d);

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
        printtime(stderr, "elapsed: ", &d);
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

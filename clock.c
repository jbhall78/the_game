#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "game.h"
#include "clock.h"

clock_driver_t  *clock_driver;

/*
 * gettimeofday time driver
 */
void
clock_gettimeofday_read(struct timeval *tv)
{
    gettimeofday(tv, NULL);
}

clock_driver_t clock_gettimeofday_driver = {
    .open  = NULL,
    .read  = clock_gettimeofday_read,
    .close = NULL,
};

/*
 * clock_gettime time driver
 */
void
clock_gettime_open(void)
{
    struct timespec tv;
    clock_getres(CLOCK_REALTIME, &tv);
    printf("clock resolution: %ld nanoseconds\n", tv.tv_nsec);
}

void
clock_gettime_read(struct timeval *tv)
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    tv->tv_sec  = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec;
    tv->tv_usec /= 1000;	// adjust nanoseconds to microseconds
}

clock_driver_t clock_gettime_driver = {
    .open  = clock_gettime_open,
    .read  = clock_gettime_read,
    .close = NULL,
};


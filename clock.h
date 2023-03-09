#ifndef _CLOCK_H
#define _CLOCK_H

typedef struct {
    void (*open)(void);
    // drivers are expected to convert their values into microseconds
    void (*read)(struct timeval *tv);
    void (*close)(void);
} clock_driver_t;

clock_driver_t *clock_driver;

/*
 * gettimeofday time driver
 */
clock_driver_t clock_gettimeofday_driver;
void clock_gettimeofday_read(struct timeval *tv);

/*
 * clock_gettime time driver
 */
clock_driver_t clock_gettime_driver;
void clock_gettime_open(void);
void clock_gettime_read(struct timeval *tv);

#endif

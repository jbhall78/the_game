#ifndef _RANDOM_H
#define _RANDOM_H

typedef struct {
    int fd;
} random_dev_urandom_cfg_t;

typedef struct {
    void *cfg;
    void (*open)(void);
    unsigned long int (*read)(void);
    void (*close)(void);
} random_driver_t;

extern random_driver_t *random_driver;

/* /dev/urandom driver */
extern random_driver_t random_dev_urandom_driver;
void random_dev_urandom_open(void);
unsigned long int random_dev_urandom_read(void);
void random_dev_urandom_close(void);

#endif

#ifndef _INPUT_H
#define _INPUT_H

#include <stdint.h>

#define INPUT_KEY_UP   1
#define INPUT_KEY_DOWN 0

#ifdef USE_EVENT
typedef struct input_event_s {
    struct timeval time;
    unsigned short type;
    unsigned short code;
    unsigned int value;
} input_event_t;

typedef struct {
    char *dev;
    // do not process events received before this time
    struct timeval start;
} input_event_cfg_t;
#endif

typedef struct {
    int data_ready;
} input_tty_cfg_t;

typedef struct {
    void *cfg;
    int fd;
    void (*open)(char *dev);
    int  (*poll)(uint32_t tv_msec, struct timeval *tv, int *c, int *type);
    void (*get_key)(struct timeval *tv, int *c, int *type);
    void (*close)();
    void (*flush)();
} input_driver_t;

input_driver_t *input_driver;


/*
 * tty input driver
 */
input_driver_t input_tty_driver;
void input_tty_open(char *device);
void input_tty_get_key(struct timeval *tv, int *key, int *type);
void input_tty_flush(void);
void input_tty_close(void);

#ifdef USE_EVENT
/*
 * /dev/input/eventX input driver
 */
input_driver_t input_event_driver;
void input_event_open(char *device);
void input_event_get_key(struct timeval *tv, int *key, int *type);
void input_event_close(void);
void input_event_flush(void);

#endif

#endif

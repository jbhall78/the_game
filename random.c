#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct {
    int fd;
} random_dev_urandom_cfg_t;

typedef struct {
    void *cfg;
    void (*open)(void);
    unsigned long int (*read)(void);
    void (*close)(void);
} random_driver_t;

random_driver_t *random_driver;

void
random_dev_urandom_open(void)
{
    int fd;
    char *filename = "/dev/urandom";
    random_dev_urandom_cfg_t *cfg;

    cfg = malloc(sizeof(random_dev_urandom_cfg_t));
    memset(cfg, 0, sizeof(random_dev_urandom_cfg_t));

    if ((fd = open(filename, O_RDONLY)) == -1) {
	fprintf(stderr,
		"ERROR: cannot open %s: %s\n", filename, strerror(errno));
	exit(1);
    }

    cfg->fd = fd;
    random_driver->cfg = cfg;
}

unsigned long int
random_dev_urandom_read(void)
{
    long int data;
    ssize_t n;
    ssize_t size = sizeof(unsigned long int);
    random_dev_urandom_cfg_t *cfg = random_driver->cfg;


    n = read(cfg->fd, &data, size);
//    printf("/dev/urandom: %d/%lu\n", n, data);
    if (n != size)
	return 100;	// in case of error return 100 instead of 0


    return data;
}

void
random_dev_urandom_close(void)
{
    random_dev_urandom_cfg_t *cfg = random_driver->cfg;
    close(cfg->fd);
    free(cfg);
}

random_driver_t random_dev_urandom_driver = {
    .cfg = NULL,
    .open = random_dev_urandom_open,
    .read = random_dev_urandom_read,
    .close = random_dev_urandom_close,
};

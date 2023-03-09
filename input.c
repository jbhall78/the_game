#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#ifdef USE_TTY
#include <fcntl.h>
#include <termios.h>
#endif

#include "game.h"
#include "clock.h"
#include "input.h"
#include "dbg.h"

/*
 * the problem with this is that this stuff doesn't really exist
 * the idea of a key/button, etc.
 *
 * because it doesn't really exist in computer-land we have a dozen
 * different symbols representing the same idea, and backwards / forward
 * compatibility between the different idea representation systems
 *
 * not sure if this makes much sense, its late.
 *
 * i guess what i care about is the following:
 *
 * 1) that which button is pressed on the keyboard makes it to the screen
 *     eg. the user presses 'a' and an a appears, SHIFT+'a' and A appears
 * 2) this action works using most languages/keyboards
 *     (some of the stranger languages can be left out)
 * 3) if the key is 'special' (eg. F1) an "<F1>" string appears
 * 4) modifiers (SHIFT/ALT/CONTROL) are able to be looked at
 * 5) VT100 key sequences are handled correctly.
 * 6) checking the value of the symbol in different formats is easy
 *     printf("%s", key->utf8); for instance
 * 7) mouse buttons & joystick buttons are treated the same
 *     as keyboard buttons
 * 8) small footprint / easy to implement (even if it is lacking a feature
 *      or two that a large library may have)
 * 9) that it maps to this programs screen driver correctly.
 * 10) input can be accepted from any system 
 *      Linux/TTY, Windows/Unicode, Raw Keyboard Interface
 */

// key_new_from_scancode
// key_new_from_tty
// key_new_from_unicode?
// key_new_from_utf8?
struct {
    unsigned int scancode;	// keyboard scan code
    unsigned int ascii;		// us/ascii value
    char *tty;                  // terminal sequence
    unsigned int unicode;	// unicode value
    char utf8[6];		// utf8 value (NULL terminated
                                //   so exclude NULLS from screen output)
    /* this is the actual text which would be displayed by this program
     * for the key code */
    char *text;
} input_key_t;

/*
 * Documentation for the Linux /dev/input stuff can be found at:
 * /usr/src/linux/Documentation/input/input.txt &
 * /usr/src/linux/Documentation/include/linux/input.h
 */
#ifdef USE_EVENT
static unsigned char input_event_kbd_xlate_tbl[] =
/*
{
    // ESC,1-9,0
    0x00, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30,
    0x2D, 0x3D, 0x7F, 0x09, // -,=,DEL,TAB
    0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69, 0x6F, 0x70, // q-p
    0x5B, 0x5D, 0x0D, 0x00, // [,],CR,CTRL
    0x61, 0x73, 0x64, 0x66, 0x67, 0x68, 0x6A, 0x6B, 0x6C // a-l
};
*/
"\000\0331234567890-=\177\t"                    /* 0x00 - 0x0f */
"qwertyuiop[]\r\000as"                          /* 0x10 - 0x1f */
"dfghjkl;'`\000\\zxcv"                          /* 0x20 - 0x2f */
"bnm,./\000*\000 \000\201\202\203\204\205"      /* 0x30 - 0x3f */
"\206\207\210\211\212\000\000789-456+1"         /* 0x40 - 0x4f */
"230\177\000\000\213\214\000\000\000\000\000\000\000\000\000\000" /* 0x50 - 0x5f */
"\r\000/";                                      /* 0x60 - 0x6f */
#endif

input_driver_t *input_driver;

#ifdef USE_TTY

int
input_generic_poll(uint32_t tv_msec, struct timeval *tv, int *key, int *type)
{
    struct timeval s;
    fd_set rfds;
    //fd_set rfds, wfds, efds;
    int err;
    input_tty_cfg_t *cfg = input_driver->cfg;

    // handle software created key events which arent select()able
    if (cfg->data_ready) {
	CALL(input_driver->get_key)(tv, key, type);
	return 1;
    }

    FD_ZERO(&rfds);
    FD_SET(input_driver->fd, &rfds);

    s.tv_sec  = tv_msec / 1000;
    s.tv_usec = (tv_msec % 1000) * 1000;

    err = select(FD_SETSIZE, &rfds, NULL, NULL, &s);
    if (err == -1) {
	// error
	abort();
    } else if (err > 0) {
	// data
	if (FD_ISSET(input_driver->fd, &rfds)) {
	    CALL(input_driver->get_key)(tv, key, type);
	    dbg("poll returned data\n");
	    return 1;
	}
    } else {
	// timeout expired
	dbg("poll timeout\n");
	return 0;
    }

    return 0;
}

/*
 * tty input driver
 */
void
input_tty_open(char *device)
{
    input_tty_cfg_t *cfg;

    // allocate configuration space
    cfg = malloc(sizeof(input_tty_cfg_t));
    assert(cfg != NULL);
    memset(cfg, 0, sizeof(input_tty_cfg_t));
    input_driver->cfg = cfg;

    input_driver->fd = fileno(stdin);

    // configure stdin for nonblocking input
    if (fcntl(input_driver->fd, F_SETFL, O_NONBLOCK) == -1) {
	fprintf(stderr,
		"ERROR: cannot set flags on input device: %s\n",
		strerror(errno));
	exit(1);
    }
}

// input_tty_get_key
// tv : time the event occured
// key: ascii key code of the event
// ev :  type of event KEY_UP, KEY_DOWN
void
input_tty_get_key(struct timeval *tv, int *key, int *type)
{
    char c;
    ssize_t n;
    input_tty_cfg_t *cfg = (input_tty_cfg_t *)input_driver->cfg;
    static char prev_c = 0;

    if (key != NULL)
	*key = 0;

    // the tty driver is limited and we are required 
    // to generate fake up/down events 
    if (prev_c != 0) {
	if (key != NULL)
	    *key   = prev_c;
	if (type != NULL)
	    *type  = INPUT_KEY_UP;
	prev_c = 0;
	cfg->data_ready = 0;
	return;
    }

    // read key from terminal
    n = read(input_driver->fd, &c, 1);

    // get current time
    if (tv != NULL)
	CALL(clock_driver->read)(tv);

    // check for errors
    if (n == -1) {
	return;
    }

    // filter unwanted input
    if (! (c > 0)) {
	return;
    }

#if 0
    // handle global keyboard commands
    if (global_keys((int)c))
	return;
#endif

    // return data to caller
    if (key != NULL)
	*key = c;
    if (type != NULL)
	*type = INPUT_KEY_DOWN;

    prev_c = c;
    cfg->data_ready = 1;
}

void
input_tty_flush(void)
{
    //input_tty_cfg_t *cfg = (input_tty_cfg_t *)input_driver->cfg;
    tcflush(input_driver->fd, TCIFLUSH);
}

void
input_tty_close(void)
{
    free(input_driver->cfg);
}

input_driver_t input_tty_driver = {
    .cfg     = NULL,
    .open    = input_tty_open,
    .poll    = input_generic_poll,
    .get_key = input_tty_get_key,
    .close   = input_tty_close,
    .flush   = input_tty_flush,
};
#endif /* USE_TTY */


#ifdef USE_EVENT
/*
 * /dev/input/eventX input driver
 */
void
input_event_open(char *device)
{
    input_event_cfg_t *cfg;
    char *dev = device;

    // allocate configuration space
    cfg = malloc(sizeof(input_event_cfg_t));
    assert(cfg != NULL);
    memset(cfg, 0, sizeof(input_event_cfg_t));
    input_driver->cfg = cfg;

    // TODO: try to automatically detect input device

    // set the device
    if (dev == NULL) {
	dev = "/dev/input/event3"; // default for my desktop machine
    }
    cfg->dev = dev;

    // open the device
    //cfg->fd = open(dev, O_RDONLY);
    input_driver->fd = open("/dev/input/event3", O_RDONLY | O_NONBLOCK);
    if (input_driver->fd == -1) {
	fprintf(stderr, "ERROR: could not open device: %s: %s\n", dev, strerror(errno));
	exit(1);
    }

    // Xorg does this, I am not completely sure why.
    //ioctl(fd, EVIOCGRAB, (void *)1);
}

/* input_get_key_event: reads keystroke from /dev/input/event* interface
 *              params: tv : time the key was pressed
 *                      key: ascii key code of the key the user pressed
 *                      type: type of key event (key_up, key_down)
 */
void
input_event_get_key(struct timeval *tv, int *key, int *type)
{
    input_event_cfg_t *cfg = (input_event_cfg_t *)input_driver->cfg;
    input_event_t event;
    ssize_t n;

    if (key != NULL)
	*key = 0;

    // check for new events
    memset(&event, 0, sizeof(input_event_t));
    n = read(input_driver->fd, &event, sizeof(input_event_t));

    if (! (n > 0)) {
	return;	// no data
    }

    // filter out events which don't interest us
    // this will probably get more complicated if we want to implment
    // our own key repeat
    //
    // additionally, it appears the documentation for the event interface
    // is incorrect. 
    // EV_KEY(0x01) && EV_REL(0x02) don't appear to be reported correctly
    // in the event.type field (maybe I am misunderstanding the meaning)
    // here is what i determined:
    // event.code  = keyboard scan code
    // event.value = 1 = KEY_DOWN  && event.value = 0 = KEY_UP
    // some other events are reported for each keypress as well.
    // not really sure what those are
    if (! ((event.type == 1) && (event.value == 1 || event.value == 0))) {
	return;
    }

    if (event.code != 0)
	dbg("EVENT: %d:%d %d %d %d\n", event.time.tv_sec, event.time.tv_usec, event.type, event.code, event.value);


#if 0
    // handle global keyboard commands
    if (global_keys(input_event_kbd_xlate_tbl[event.code]))
	return;
#endif

    // filter out events in the past
    if (event.time.tv_sec < cfg->start.tv_sec ||
	    (event.time.tv_sec == cfg->start.tv_sec &&
	     event.time.tv_usec < cfg->start.tv_usec)) {
	return;
    }

    // we found an event which interests us,
    // return it back to the caller
    if (key != NULL)
	*key        = input_event_kbd_xlate_tbl[event.code];
    if (type != NULL) {
	if (event.value == 0x01)
	    *type = INPUT_KEY_DOWN;
	else if (event.value == 0x00)
	    *type = INPUT_KEY_UP;
	else
	    abort();
    }
    if (tv != NULL) {
	tv->tv_sec  = event.time.tv_sec;
	tv->tv_usec = event.time.tv_usec;
    }
}

void
input_event_close(void)
{
    //input_event_cfg_t *cfg = (input_event_cfg_t *)input_driver->cfg;

    tcflush(fileno(stdin), TCIFLUSH);

    close(input_driver->fd);
    free(input_driver->cfg);
}

void
input_event_flush(void)
{
    struct timeval tv;
    input_event_cfg_t *cfg = (input_event_cfg_t *)input_driver->cfg;

    CALL(clock_driver->read)(&tv);
    memcpy(&cfg->start, &tv, sizeof(struct timeval));
}

// careful using this driver in X, it doesn't care which window is focused
// it reads raw keyboard events
input_driver_t input_event_driver = {
    .cfg     = NULL,
    .open    = input_event_open,
    .poll    = input_generic_poll,
    .get_key = input_event_get_key,
    .close   = input_event_close,
    .flush   = input_event_flush,
};
#endif /* USE_EVENT */

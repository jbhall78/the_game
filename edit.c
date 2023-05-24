/*
 * Name: edit
 * Description: game animation editor
 * Author: Jason Hall <jbhall78@gmail.com>
 * Copyright: (C) 2011 Jason Hall <jbhall78@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#ifdef USE_REALTIME
#include <sched.h>
#endif

#include "game.h"	// TODO: remove this dependency & replace with edit.h
#include "clock.h"
#include "input.h"
#include "random.h"
#include "screen.h"
#include "terminal.h"
#include "widget.h"
#include "dbg.h"

screen_t *screen;
widget_t *root;
unsigned int mouse_x = 0, mouse_y = 0;
input_driver_t *input_driver;
extern input_driver_t input_tty_driver;
extern input_driver_t input_event_driver;
random_driver_t *random_driver;

void
edit_background_draw(screen_t *s, widget_t *w)
{
    int i, j;
    screen_special_chars_t *ssc = screen_special_chars;
    int x, y;

    x = y = 0;

    // upper left -> upper right
    screen_set_char(s, x, y, ssc->ul);
    for (i = 1; i < w->width-1; i++)
	screen_set_char(s, x+i, y, ssc->hline);
    screen_set_char(s, x+w->width-1, y, ssc->ur);

    // vertical lines on both sides
    for (j = 1; j < w->height-1; j++) {
	screen_set_char(s, x, y+j, ssc->vline);
	screen_set_char(s, x+w->width-1, y+j, ssc->vline);
    }

    // bottom left -> bottom right
    screen_set_char(s, x, y+w->height-2, ssc->ll);
    for (i = 1; i < w->width-1; i++)
	screen_set_char(s, x+i, y+w->height-2, ssc->hline);
    screen_set_char(s, x+w->width-1, y+w->height-2, ssc->lr);
}

void
edit_shutdown(void)
{
    CALL(term_driver->close)();
    CALL(input_driver->close)();
    CALL(clock_driver->close)();
}

void
edit_key(int key, int type)
{
    widget_key(root, key, type);
}

void
edit_mmotion(unsigned int x, unsigned int y)
{
    mouse_x = x;
    mouse_y = -1;
}

/*
 * edit_mbutton:
 * num: button number
 * state: 1 for down, 0 for up
 */
void
edit_mbutton(unsigned int num, unsigned int state)
{

}

/*
 * BUG: There is a memory leak somewhere, which shows up if this function
 *      is called repeatedly. To make it show up, comment out the
 *      data_ready = 0 line in input.c:input_tty_get_key()
 *
 *      Need to investigate further to determine whats wrong.
 */
void
edit_msleep(uint32_t tv_msec)
{
    int key, type;
    struct timeval before, after;
    int err;

    for (;;) {
	CALL(clock_driver->read)(&before);
	err = input_driver->poll(tv_msec, NULL, &key, &type);
	if (err == -1) {
	    abort();
	} else if (err > 0) {
	    edit_key(key, type);
	    widget_draw(screen, root);
	    screen_flip(screen);

	    CALL(clock_driver->read)(&after);

	    double tm = (((after.tv_sec - before.tv_sec) * 1000000)
		    + (after.tv_usec - before.tv_usec)) / 1000;
	    // msec == total time slept
	    uint32_t msec = (uint32_t)tm;

	    if (msec >= tv_msec) {
		break;
	    }
	    tv_msec -= msec;
	} else {
	    break;
	}
    }

    return;

#if 0
    int inc = 10;  // increment by 10ms
    for (i = 0; i < tv_msec; i += inc) {
        usleep(inc * 1000);
	do {
	    CALL(input_driver->getkey)(NULL, &key);
	    if (key != 0)
		edit_key(key);
	} while (key != 0);
    }

#endif
}


void
usage(char **argv)
{
    char *s = strrchr(argv[0], '/') ? basename(argv[0]) : argv[0];

    fprintf(stderr, "\n");
    fprintf(stderr, "usage: %s [filename]\n", s);
    fprintf(stderr, "\n");

    exit(1);
}

/* BUG: there is some bug in ncurses
        which causes this to crash somewhat randomly
   speculation: maybe the signal handler isnt returning before it gets
                called again?
*/
void
sig_winch(int sig)
{
    screen->foreground = TEXT_COLOR_WHITE;
    screen->background = TEXT_COLOR_RED;

    CALL(term_driver->resize)();
    screen_resize(screen);
}

int
main(int argc, char **argv)
{
    int err;
    int i;
    slist_t *files;

    // process command line options
    for (i = argc-1; i > 0; i--) {
    	struct stat st;
	char *filename = argv[i];
	unsigned int len = strlen(argv[i]);

	// this program doesn't really have any command line options
	// but process -h correctly and display the proper error messages
	if (argv[i][0] == '-') {
	    if (len > 1) {
	        if (argv[i][1] == 'h')
    		    usage(argv);
	        else {
		    fprintf(stderr,
			    "ERROR: unknown option: '%c'\n", argv[i][1]);
		    usage(argv);
		}
	    } else {
		usage(argv);
	    }
	}

	// try to stat the file to ensure we were given a
	// correct command line option
	err = stat(filename, &st);
	if (err == -1) {
	    fprintf(stderr,
		    "ERROR: cannot stat file: %s: %s\n",
		    filename,
		    strerror(errno));
	    usage(argv);
	}

	// add the file to the list of files we are going to be working on
	slist_t *file = slist_new();
	file->data = strdup(argv[i]);
	files = slist_add_start(files, file);
    }

    // initialize drivers
    // ------------------
    // we do this in a less configurable way then the game
    // as a lot of things here are less critical
    clock_driver  = &clock_gettimeofday_driver;
    input_driver  = &input_tty_driver;
    //input_driver  = &input_event_driver;
#if USE_INT10
    term_driver   = &term_int10_driver;
#endif
#if USE_CURSES
    term_driver   = &term_curses_driver;
#endif
    random_driver = &random_dev_urandom_driver;

#if 0
    printf("loading: data/01.txt\n");
    anim_t *anim = anim_load("data/01.txt");
    printf("frame count: %d\n", anim->n_frames);
    printf("alpha char: %c\n", anim->char_alpha);
    printf("spinner char: %c\n", anim->char_spinner);
    slist_t *item;
    int frame = 0;
    for (item = anim->frames; item; item = item->next) {
	anim_frame_t *af = (anim_frame_t *)item->data;
	char buf[BUFSIZ];
	int line;

	printf("frame: %d\n", frame);
	printf("frame width: %d\n", af->width);
	printf("frame height: %d\n", af->height);
	printf("image:\n");

	for (line = 0; line < af->height; line++) {
	    memset(buf, 0, sizeof(buf));
	    memcpy(buf, af->data + (line * af->width), af->width);
	    printf("[%s]\n", buf);
	}
	printf("\n");
	frame++;
    }
    exit(0);
#endif

    dbg_open();

    // initialize the time driver
    CALL(clock_driver->open)();

    // initialize the input driver
    CALL(input_driver->open)(NULL);

    // initialize the random number generator
    CALL(random_driver->open)();

    // initialize the terminal
    //int opts = TERMINAL_MENU_BAR | TERMINAL_STATUS_BAR;
    //int opts = TERMINAL_STATUS_BAR | TERMINAL_MOUSE;
    //int opts = TERMINAL_MOUSE;
    int opts = 0;
    CALL(term_driver->open)(opts);
//    term_driver->mouse_motion_handler = edit_mbutton;
//    term_driver->mouse_button_handler = edit_mmotion;

    // initialize the virtual screen
    screen = screen_init();

    // configure shutdown handler
    err = atexit(edit_shutdown);
    assert(err == 0);

    // disable CTRL+C
    //signal(SIGINT, SIG_IGN);

    // handle window resizes (buggy)
    signal(SIGWINCH, sig_winch);

    // create test form
    root            = widget_new(NULL, WIDGET_ROOT,
	                         screen->width, screen->height, 0, 0);
    widget_t *win   = widget_new(root, WIDGET_WINDOW,
	                         screen->width, screen->height, 0, 0);
    widget_t *label, *input, *button, *checkbox, *user, *status_bar, *select;

    user = widget_new(win, WIDGET_USER, screen->width, screen->height, 0, 0);
    widget_user_set_handlers(user, edit_background_draw, NULL);

    label = widget_new(win,  WIDGET_TEXT_LABEL, 10, 1, 3, 3);
    widget_text_label_set_alignment(label, WIDGET_RIGHT);
    widget_text_label_set_text(label, "input 1:");

    input = widget_new(win,  WIDGET_TEXT_INPUT, 10, 1, 16, 3);
    widget_text_input_set_alignment(input, WIDGET_LEFT);

    label = widget_new(win,  WIDGET_TEXT_LABEL, 10, 1, 3, 6);
    widget_text_label_set_alignment(label, WIDGET_RIGHT);
    widget_text_label_set_text(label, "input 2:");

    input = widget_new(win,  WIDGET_TEXT_INPUT, 10, 1, 16, 6);
    widget_text_input_set_alignment(input, WIDGET_LEFT);

    label = widget_new(win,  WIDGET_TEXT_LABEL, 10, 1, 3, 9);
    widget_text_label_set_alignment(label, WIDGET_RIGHT);
    widget_text_label_set_text(label, "checkbox 1:");

    checkbox = widget_new(win, WIDGET_CHECKBOX, 3, 1, 16, 9);
    widget_checkbox_set_checked(checkbox, 1);

    label = widget_new(win,  WIDGET_TEXT_LABEL, 10, 1, 3, 12);
    widget_text_label_set_alignment(label, WIDGET_RIGHT);
    widget_text_label_set_text(label, "select 1:");
    select = widget_new(win, WIDGET_SELECT, 3, 1, 16, 12);

    button = widget_new(win, WIDGET_BUTTON, 6, 3,
	    screen->width - 13, screen->height - 7);
    widget_button_set_label(button, "OK");

    status_bar = widget_new(win, WIDGET_STATUS_BAR,
	    screen->width, 1, 0, screen->height - 1);
    widget_status_bar_set_text(status_bar, "Ready.");

    screen_clear(screen);
    screen_flip(screen);

    // main loop
    i = 0;
    CALL(input_driver->flush)();

    /* SERIOUS BUG: Something is overwriting bits of the widget tree.
       I tried debugging it with gdb and it ended up causing gdb to crash.
       Due to the names of the variables and the way it crashed I suspect it
       was Federal Government's backdoor causing the bug and am not going to
       wasting my time to track it down if it was someone harassing me,
       since they are constantly trying to distract me when I am trying to
       be productive. I reordered some variables in the widget_t structure
       to fix it the effects.  But haven't found the actual cause.
       If it was actually my fault, I'm sure I will run across it sooner or
       later.
     */
    while (1) {
#if 0
	screen_set_text_attribs(screen,
		TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_ATTRIB_NONE);
	screen_clear(screen);
#endif

	widget_draw(screen, root);

	//CALL(term_driver->check_events)();
	//screen_printf(screen, 1, 20, "Iteration: [%d]", i);
	//screen_printf(screen, 1, 23, "Mouse Position: [%d, %d]", mouse_x, mouse_y);

	screen_flip(screen);
	i++;
	edit_msleep(5000);
    }

    return 0;
}


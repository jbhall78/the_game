/* 
 * Namt: reaction-test
 * Description: A terminal program which tests a person's reaction time
 * Author: Jason Hall <jbhall78@gmail.com>
 * Copyright: (C) Jason Hall 2011
 *	(will bsd it when finished)
 */
/*
 * Asset Loading
 * Doublebuffering
 * Blitting
 *   require specific button press or combination
 *   Mouse support
 *   Color adjustment
 *   Audio Alerts
 *
 *   X11 or SDL
 *   Windows Port
 *   FreeBSD Port
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

#ifdef USE_REALTIME
#include <sched.h>
#endif


#include "game.h"
#include "clock.h"
#include "input.h"
#include "random.h"
#include "screen.h"
#include "terminal.h"
#include "widget.h"
#include "dbg.h"
#include "mkstr.h"

game_data_t *game_data;

int
game_keys(int key, int type)
{
    switch (key) {
	// handle global  keyboard commands
	case 0x1B:	// ESCAPE KEY
	    exit(0);
	    break;
	default:
#if 0
	    {
		term_curses_cfg_t *cfg = term_driver->cfg;
		mvwprintw(cfg->win_main, 0, 1, "DEBUG: key: %d    ", key);
	    }
#endif
	    return 0;
	    break;
    }

    return 1;
}

int
log_open(char *filename)
{
    int fd;

    fd = open(filename, O_CREAT|O_APPEND|O_WRONLY, 0640);
    if (fd == -1) {
	fprintf(stderr, "ERROR: cannot open logfile: %s\n", strerror(errno));
	exit(1);
    }

    return fd;
}

void
log_write(int fd, time_t date_time, int reaction_time, char *delim, char *quote)
{
    char line_buf[BUFSIZ];
    char num_buf[BUFSIZ];
    int err, len;

    memset(line_buf, 0, sizeof(line_buf));

    // add date & time
    if (quote != NULL)
	strncat(line_buf, quote, sizeof(line_buf));
    snprintf(num_buf, sizeof(num_buf), "%ld", date_time);
    strncat(line_buf, num_buf, sizeof(line_buf));
    if (quote != NULL)
	strncat(line_buf, quote, sizeof(line_buf));

    // add delimeter
    strncat(line_buf, delim, sizeof(line_buf));

    // add reaction time
    if (quote != NULL)
	strncat(line_buf, quote, sizeof(line_buf));
    snprintf(num_buf, sizeof(num_buf), "%d", reaction_time);
    strncat(line_buf, num_buf, sizeof(line_buf));
    if (quote != NULL)
	strncat(line_buf, quote, sizeof(line_buf));

    // add newline
    strncat(line_buf, "\n", 1);

    len = strlen(line_buf);
    err = write(fd, line_buf, len);
    if (err != len) {
	fprintf(stderr, "ERROR: cannot write to log file: %s\n", strerror(errno));
	exit(1);
    }
}

void
game_intro(screen_t *screen)
{
    int ix, iy, cx, cy;
    int fx, fy;
    int itime = 3000;
    int istep, isteps;
    anim_frame_t *frame;

    screen_set_text_attribs(screen, TEXT_COLOR_WHITE, TEXT_COLOR_BLACK, TEXT_ATTRIB_NONE);
    anim_t *logo = anim_load("data/08.txt");

    // run frame 1
    frame = anim_frame(logo, 0);

    cx = (screen->width / 2) - (frame->width / 2) - 3;
    cy = (screen->height / 2) - (frame->height / 2);

    isteps = 0;
    for (iy = -frame->height; iy < cy; iy++) {
	isteps++;
    }
    istep = itime / isteps;

    for (iy = -frame->height; iy < cy; iy++) {
	screen_clear(screen);
	screen_set_text_attribs(screen, TEXT_COLOR_YELLOW, TEXT_COLOR_BLACK, TEXT_ATTRIB_NONE);
	screen_blit(screen, logo, 0, cx, iy);
	fx = cx;
	fy = iy;
	screen_flip(screen);
	game_msleep(istep);
    }

    game_msleep(1000);
    screen_clear(screen);
    screen_blit(screen, logo, 1, fx, fy);
    screen_flip(screen);

    game_msleep(3000);

    // run frame 2
    frame = anim_frame(logo, 1);
    cx = (screen->width / 2) - (frame->width / 2) - 3;
    cy = (screen->height / 2) - (frame->height / 2);

    isteps = 0;
    for (ix = screen->width + frame->width; ix > cx; ix--) {
	isteps++;
    }
    istep = itime / isteps;
    for (ix = screen->width + frame->width; ix > cx; ix--) {
	fx--;

	screen_clear(screen);
	screen_set_text_attribs(screen, TEXT_COLOR_YELLOW, TEXT_COLOR_BLACK, TEXT_ATTRIB_NONE);
	screen_blit(screen, logo, 1, fx, fy);
	screen_set_text_attribs(screen, TEXT_COLOR_GREEN, TEXT_COLOR_BLACK, TEXT_ATTRIB_NONE);
	screen_blit(screen, logo, 2, ix, cy);

	screen_flip(screen);
	game_msleep(istep);
    }
    ix++;

    game_msleep(1000);
    screen_clear(screen);
    screen_blit(screen, logo, 2, ix, cy);
    screen_set_text_attribs(screen, TEXT_COLOR_RED, TEXT_COLOR_BLACK, TEXT_ATTRIB_BLINK);
    screen_blit(screen, logo, 3, ix, cy);
    screen_set_text_attribs(screen, TEXT_COLOR_WHITE, TEXT_COLOR_BLACK, TEXT_ATTRIB_NONE);
    screen_flip(screen);

    game_msleep(5000);
    screen_clear(screen);
    screen_flip(screen);
}

void
game_update(void)
{
    slist_t *item;
    double total = 0.0;
    int items = 0;

    for (item = game_data->reaction_list; item; item = item->next) {
	reaction_t *reaction = item->data;

	total += (double)reaction->reaction_time;
	items++;
    }

    game_data->reactions = items;
    game_data->average = (items > 1) ? total / (double)items : total;

    return;
}

/* game_msleep: sleep during the game but peridocally check devices for input */
/* tv_msec: time in ms to sleep */
void
game_msleep(unsigned int tv_msec)
{
    unsigned int i;
    int key, type;
    int inc = 10;  // increment by 10ms
    for (i = 0; i < tv_msec; i += inc) {
	usleep(inc * 1000);
	CALL(input_driver->get_key)(NULL, &key, &type);
	game_keys(key, type);
    }
}

void
game_shutdown(void)
{
    CALL(term_driver->close)();
    CALL(input_driver->close)();
    CALL(clock_driver->close)();
}

void
usage(char **argv)
{
    char *s = strrchr(argv[0], '/') ? basename(argv[0]) : argv[0];

    fprintf(stderr, "\n");
    fprintf(stderr, "usage: %s [-r] [-i <driver>] [-l <logfile>] [-c <driver>]\n", s);
    fprintf(stderr, "\t-c <gettimeofday|clock_gettime>\n"
		"\t\tselect clock driver [default=gettimeofday]\n");
    fprintf(stderr, "\t-i <tty|event[:device=/dev/input/eventX]>\n"
		"\t\tselect input driver [default=tty]\n"
		"\t\t\tdevice: device file to use\n\n");
    fprintf(stderr, "\t-l <logfile[:delim=<delimiter>][:quote=<quotechar>]>\n"
		"\t\tlog reactions to <logfile> [default=none]\n"
		"\t\t  delimiter: delimiter to use in the CSV log file "
			"[default=, ]\n"
		"\t\t  quote: quote character to use for each row item " 
			"[default=\"]\n\n");
#ifdef USE_REALTIME
    fprintf(stderr, "\t-r\tenable real time priority [default=off]\n\n");
#endif
    fprintf(stderr, "\t-s\tskip intro screen [default=off]\n\n");
#ifdef USE_CURSES
    fprintf(stderr, "\t-t <ncurses|tty>\n"
		"\t\tselect terminal driver [default=ncurses]\n");
#endif
    fprintf(stderr, "\n");

    exit(1);
}

/* this function could probably stand to be condensed, but i wrote it quickly */
/* i also have no clue why i did this in this manner, since : cannot be used
   as a delimiter and it is a pretty common delimiter. this needs to be fixed */
/* this function uses a lot of stack space too */
void
parse_log_options(char **argv, char *arg,
	char **opt_logfile, char **opt_delim, char **opt_quote)
{
    char str[BUFSIZ];
    char option[BUFSIZ];
    char *p, *r, *q;

    // truncate oversized input
    memset(str, 0, sizeof(str));
    strncpy(str, arg, BUFSIZ-1);

    memset(option, 0, sizeof(option));

//    printf("parsing %s\n", str);
    p = strchr(str, ':');

    if (p == NULL) {
//	printf("logfile: %s\n", str);
	*opt_logfile = strdup(str);
	return;
    } else {
	strncpy(option, str, p - str); 
	//printf("option: %s\n", option);
	p++;

//	printf("logfile: %s\n", option);
	*opt_logfile = strdup(option);

	// split options loop
	for (;;) {
	    char key_val[BUFSIZ];
	    char key[BUFSIZ];
	    char val[BUFSIZ];
	    int len;

	    // determine length of key_val string
	    r = strchr(p, ':');
	    if (r == NULL) {
		len = strlen(p);
	    } else {
		len = r - p;
	    }

	    // if empty
	    if (! (len > 0)) {
		// handle empty key_val segments correctly
		if (strlen(p) > 0) {
		    p++;
		    continue;
		} else {
		    break;
		}
	    }
	    memset(key_val, 0, sizeof(key_val));
	    strncpy(key_val, p, len);

	    //printf("key_val: %s\n", key_val);
	    //no boolean options

	    q = strchr(key_val, '=');
	    memset(key, 0, sizeof(key));
	    memset(val, 0, sizeof(val));
	    if (q == NULL) {
		len = strlen(key_val);
		strncpy(key, key_val, len);
	    } else {
		len = q - key_val;
		strncpy(key, key_val, len);
		strncpy(val, q + 1, strlen(q));

//		printf("key = [%s]  val = [%s]\n", key, val);
		if (strcmp(key, "delim") == 0) {
		    *opt_delim = strdup(val);
		} else if (strcmp(key, "quote") == 0) {
		    *opt_quote = strdup(val);
		} else {
		    fprintf(stderr, "ERROR: invalid key specified: %s\n", key);
		    usage(argv);
		    exit(1);
		}
	    }

	    // special case for last option
	    if (r == NULL)
		break;
	    p = r;
	    p++;
	}
    }
}

int
main(int argc, char **argv)
{
    int log_fd;
    struct timeval before, after;
    int err;
    int i;

    /* command line options */
#ifdef USE_REALTIME
    int opt_realtime    = 0;
#endif
    char *opt_logfile   = NULL;
    char *opt_delim     = ", ";
    char *opt_quote     = "\"";
    int  opt_skip_intro = 0;

    char *input_event_device = NULL;

    /* initialize drivers */
    clock_driver  = &clock_gettimeofday_driver;
    input_driver  = &input_tty_driver;
#if USE_INT10
    term_driver   = &term_int10_driver;
#endif
#ifdef USE_CURSES
    term_driver   = &term_curses_driver;
#else
    term_driver   = &term_tty_driver;
#endif
    random_driver = &random_dev_urandom_driver;
    screen_t *screen;


    for (i = 1; i < argc; i++) {
	int len  = strlen(argv[i]);
	if (len > 1) {
	    char opt = argv[i][1];
	    char *arg = NULL;
	    if (argc > i) {
		arg = argv[i+1];
	    }
	    switch(opt) {
		case 'c':
		    if (! arg) {
			fprintf(stderr, "ERROR: "
				"no argument supplied with -c option\n");
			usage(argv);
		    } else {
			i++;
		    }
		    if (strcmp(arg, "gettimeofday") == 0) {
			clock_driver = &clock_gettimeofday_driver;
		    } else if (strcmp(arg, "clock_gettime") == 0) {
			clock_driver = &clock_gettime_driver;
		    } else {
			fprintf(stderr, "ERROR: "
				"unknown time driver: %s\n", arg);
		    }
		    break;
		case 'i':
		    if (! arg) {
			fprintf(stderr, "ERROR: "
				"no argument supplied with -i option\n");
			usage(argv);
		    } else {
			i++;
		    }
		    if (strcmp(arg, "tty") == 0) {
			input_driver = &input_tty_driver;
		    } else if (strncmp(arg, "event", 5) == 0) {
			char *p;

			input_driver = &input_event_driver;

			if (len > 6 && arg[5] == ':') {
			    p = &arg[6];
			    if (strncmp(p, "device", 6) == 0) {
				if (strchr(p, '=') == 0) {
				    if (++p != NULL) {
					input_event_device = strdup(p);
				    }
				}
			    }
			}
		    } else {
			fprintf(stderr, "ERROR: "
				"unknown input driver: %s\n", arg);
		    }
		    break;
		case 'l':
		    if (! arg) {
			fprintf(stderr, "ERROR: "
				"no argument supplied with -l option\n");
			usage(argv);
		    } else {
			i++;
		    }
		    parse_log_options(argv, arg,
			    &opt_logfile, &opt_quote, &opt_delim);
		    break;
#ifdef USE_REALTIME
		case 'r':
		    opt_realtime ^= 1;
		    break;
#endif
		case 's':
		    opt_skip_intro ^= 1;
		    break;
#ifdef USE_CURSES
		case 't':
		    if (! arg) {
			fprintf(stderr, "ERROR: "
				"no argument supplied with -t option\n");
			usage(argv);
		    } else {
			i++;
		    }
		    if (strcmp(arg, "ncurses") == 0 || strcmp(arg, "curses") == 0) {
			term_driver  = &term_curses_driver;
		    } else if (strcmp(arg, "tty") == 0) {
			term_driver  = &term_tty_driver;
		    } else {
			fprintf(stderr, "ERROR: "
				"unknown time driver: %s\n", arg);
		    }
		    break;
#endif
		default:
		    fprintf(stderr, "ERROR: unknown option: '%c'\n", opt);
		    usage(argv);
		    break;
	    }
	}
    }

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

#ifdef USE_REALTIME
    if (opt_realtime) {
	pid_t pid = getpid();
	struct sched_param sparam;

	sparam.sched_priority = sched_get_priority_max(SCHED_FIFO);
	printf(">>> switching to real time scheduling "
		"(SCHED_FIFO %d priority)\n", sparam.sched_priority);
	err = sched_setscheduler(pid, SCHED_FIFO, &sparam);
	if (err == -1) {
	    fprintf(stderr, "ERROR: "
		    "unable to set scheduler parameters: %s\n\n",
		    strerror(errno));
	    exit(1);
	}
    }
#endif

    dbg_open();

    // initialize the time driver
    CALL(clock_driver->open)();

    // initialize the input driver
    CALL(input_driver->open)(input_event_device);

    // initialize the random number generator
    CALL(random_driver->open)();

    if (opt_logfile != NULL) {
	log_fd = log_open(opt_logfile);
    }

    // initialize the terminal
    //int opts = TERMINAL_STATUS_BAR;
    int opts = 0;
    CALL(term_driver->open)(opts);

    // initialize the virtual screen
    screen = screen_init();

    // configure shutdown handler
    err = atexit(game_shutdown);
    assert(err == 0);

    // disable CTRL+C
    //signal(SIGINT, SIG_IGN);

    game_data = malloc(sizeof(game_data_t));
    memset(game_data, 0, sizeof(game_data_t));

    if (! opt_skip_intro)
	game_intro(screen);

    widget_t *root  = widget_new(NULL, WIDGET_ROOT,
	                         screen->width, screen->height, 0, 0);
    widget_t *win   = widget_new(root, WIDGET_WINDOW,
	                         screen->width, screen->height, 0, 0);
    widget_t *label_help, *label_reaction, *button_alert, *status_bar;

    label_help = widget_new(win,  WIDGET_TEXT_LABEL, screen->width, 1, 1, 1);
    widget_text_label_set_text(label_help,
	    "Press a button when <NOW> is displayed.");


    label_reaction = widget_new(win, WIDGET_TEXT_LABEL,
	    screen->width, 3, 1, 3);
    widget_text_label_set_text(label_reaction, NULL);

    status_bar = widget_new(win, WIDGET_STATUS_BAR,
	    screen->width, 1, 0, screen->height-1);
    widget_status_bar_set_text(status_bar, "Press <ESCAPE> to exit.");
    widget_set_attribs(status_bar, WIDGET_ATTRIB_ACTIVE, TEXT_COLOR_RED, TEXT_COLOR_WHITE, 0);

 
    button_alert = widget_new(win, WIDGET_BUTTON, 20, 5,
	    ((screen->width/2) - ((20/2) - 1)),
	    ((screen->height/2) - ((5/2) - 1)));
    button_alert->visible = 0;
    button_alert->can_focus = 0;
    widget_button_set_label(button_alert, "<NOW>");
    widget_set_attribs(button_alert, WIDGET_ATTRIB_ACTIVE, TEXT_COLOR_RED, TEXT_COLOR_WHITE, 0);
    widget_set_attribs(button_alert, WIDGET_ATTRIB_BORDER_ACTIVE, TEXT_COLOR_RED, TEXT_COLOR_WHITE, 0);

/*
    screen_set_text_attribs(screen, TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_ATTRIB_NONE);
    screen_clear(screen);
    screen_flip(screen);
*/

    // main loop
    while (1) {
	static int iteration = 0;
	unsigned long int rnd = 0;
	int rnd_min = 500;  // 500ms
	int rnd_max = 5000; // 5sec
	int rhythm[] = {
	    0,3,3,3,2,2,2,2,2,2,1,1,1,1,1
	};

	widget_draw(screen, root);
	screen_flip(screen);

	//CALL(term_driver->draw_before)();

	// sleep for a short but random amount of time
	// game_msleep((random() % 4 + 1) * 1000); 
	rnd = random_driver->read();

	// scale number using floating point numbers (hopefully accurate)
	double ra = (double)ULONG_MAX;
	double rb = (double)rnd;
	double rc = (double)rnd_max;
	double rd = rc / ra * rb;
	rnd = (unsigned long int)rd;

	if (rnd < rnd_min)
	    rnd = rnd_min;
	if (rnd > rnd_max)
	    rnd = rnd_max;

	if (iteration < sizeof(rhythm)/sizeof(rhythm[0]))
	    rnd += rhythm[iteration] * 1000;
	iteration++;

	game_msleep(rnd);

	CALL(input_driver->flush)();

	button_alert->visible = 1;
	widget_draw(screen, root);
	screen_flip(screen);

	//CALL(term_driver->draw_alert)();

	CALL(clock_driver->read)(&before);

	// input loop
	for (;;) {
	    int key, type;
	    CALL(input_driver->get_key)(&after, &key, &type);

	    if (key == 0) {
		continue;
	    }

	    if (type == INPUT_KEY_UP)
		continue;

	    double dd = (double)before.tv_sec;
	    double de = (double)after.tv_sec;
	    double df = de - dd;

	    double dg = (double)before.tv_usec;
	    double dh = (double)after.tv_usec;
	    double dj = dh - dg;

	    double dk = dj * 0.000001;
	    double dl = df + dk;
	    double dr = dl * 1000;
	    int reaction_time = (int)dr;

	    // create reaction item
	    reaction_t *reaction = malloc(sizeof(reaction_t));
	    memset(reaction, 0, sizeof(reaction_t));
	    reaction->date_time     = de;
	    reaction->reaction_time = reaction_time;
	    game_data->latest_reaction = dr;

	    // add reaction item to list
	    slist_t *item = slist_new();
	    item->data = reaction;
	    game_data->reaction_list = slist_add_end(game_data->reaction_list, item);

	    // write information to logfile
	    if (opt_logfile != NULL)
		log_write(log_fd, de, reaction_time, opt_delim, opt_quote);

	    game_update();

	    button_alert->visible = 0;

	    char *str = mkstr(
		    "Your reaction time: %-4.0f milliseconds\n"
		    "Your average reaction time: %-4.0f milliseconds\n"
		    "Reactions Tested: %d",
		    game_data->latest_reaction,
		    game_data->average,
		    game_data->reactions);

	    widget_text_label_set_text(label_reaction, str);
	    free(str);


	    //CALL(term_driver->draw_after)();

	    // pause for the next round
	    // game_msleep(3 * 1000);
	    break;
	}
    }

    return 0;
}

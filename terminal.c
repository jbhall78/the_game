#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#ifdef USE_TTY
#include <termios.h>
#include <sys/ioctl.h>
#endif /* USE_TTY */

#ifdef USE_CURSES
#include <ncursesw/ncurses.h>
#endif /* USE_CURSES */

#include "game.h"
#include "screen.h"
#include "terminal.h"

term_driver_t *term_driver;


#ifdef USE_CURSES
static int
term_curses_xlate_color(int text_color)
{
    int color_tbl[] = {
	COLOR_BLACK,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_CYAN,
	COLOR_BLACK,
	COLOR_BLACK,
	COLOR_WHITE
    };
    int ret;

    if (text_color < 0 || text_color > 9)
	text_color = 0;

    ret = color_tbl[text_color];
//    fprintf(stderr, "translated: %d to %d\n", text_color, ret);

    return ret;
}

static int
color(int fg, int bg)
{
    int y, x, idx;

    // a color to index table could be used here for a
    // trivial performance gain
    idx = 0;
    for (y = 0; y < 10; y++) {
	if (y == 7 || y == 8)
	    continue;

	for (x = 0; x < 10; x++) {
	    if (x == 7 || x == 8)
		continue;

	    if (y == bg && x == fg)
		return COLOR_PAIR(idx);

	    idx++;
	}
    }

    // we were specified an invalid color
    return COLOR_PAIR(0);
}

void
term_ncurses_set_specials(screen_special_chars_t *ssc)
{
    ssc->vline      = ACS_VLINE;
    ssc->hline      = ACS_HLINE;
    ssc->ul         = ACS_ULCORNER;
    ssc->ll         = ACS_LLCORNER;
    ssc->ur         = ACS_URCORNER;
    ssc->lr         = ACS_LRCORNER;
    ssc->vh_cross   = ACS_PLUS;
    ssc->shade0     = ACS_BLOCK;
    ssc->shade1     = ACS_BOARD;
    ssc->shade2     = ACS_CKBOARD;
    ssc->shade3     = ACS_CKBOARD;
    ssc->diamond    = ACS_DIAMOND;
    ssc->bullet     = ACS_BULLET;
    ssc->degree     = ACS_DEGREE;
    ssc->plus_minus = ACS_PLMINUS;
}

void
term_curses_open(int opts)
{
    WINDOW *win_main, *win_status = NULL, *win_menubar = NULL;
    term_curses_cfg_t *cfg;
    int i, j;
    screen_special_chars_t *ssc;

    ssc = malloc(sizeof(screen_special_chars_t));
    memset(ssc, 0, sizeof(screen_special_chars_t));


    cfg = malloc(sizeof(term_curses_cfg_t));
    assert(cfg != NULL);
    memset(cfg, 0, sizeof(term_curses_cfg_t));

    initscr();
    cbreak();
    noecho();
    nonl();
//    clear();

    if (has_colors() != TRUE) {
	fprintf(stderr, "Your terminal does not support color\n");
	exit(1);
    }

    start_color();

    term_ncurses_set_specials(ssc);

    if (opts & CURSES_MOUSE) {
	mmask_t m2, m1 = REPORT_MOUSE_POSITION;
	// enable mouse
	m2 = mousemask(m1, NULL);
	if (m2 != m1 || has_mouse() != TRUE) {
	    fprintf(stderr, "Your terminal does not support mouse input\n");
	    exit(1);
	}

    }
    // is it possible that this could != 64? :(
    //fprintf(stderr, "color pairs: %d\n", COLOR_PAIRS);

    // initialize colors
    int idx, fg, bg;
    idx = fg = bg = 0;
    for (j = 0; j < 10; j++) {
	if (j == 7 || j == 8)
	    continue;

	for (i = 0; i < 10; i++) {
	    if (i == 7 || i == 8)
		continue;

	    // life could be simpler
	    //idx = (j * 10) + i;
	    bg = term_curses_xlate_color(j);
	    fg = term_curses_xlate_color(i);

	    init_pair(idx, fg, bg);
	    idx++;
//	    fprintf(stderr, "%2d = %2d %2d  %d %d\n", idx, xbg, xfg, j, i);
	}
    }

    cfg->win_main_height = LINES;
    int y = 0;
    if (opts & CURSES_MENU_BAR) {
	cfg->win_main_height--;
	y++;
    }
    if (opts & CURSES_STATUS_BAR)
	cfg->win_main_height--;

    // create & draw main window
    win_main = newwin(cfg->win_main_height, COLS, y, 0);
    wattrset(win_main,   color(TEXT_COLOR_WHITE, TEXT_COLOR_BLUE));
    wnoutrefresh(win_main);

    // create & draw menubar bar
    if (opts & CURSES_MENU_BAR) {
	win_menubar = newwin(1, COLS, 0, 0);
	wattrset(win_menubar, color(TEXT_COLOR_BLACK, TEXT_COLOR_WHITE));
	whline(win_menubar, ' ', COLS);
	wnoutrefresh(win_menubar);
    }

    // create & draw status bar
    if (opts & CURSES_STATUS_BAR) {
	win_status = newwin(1, COLS, LINES - 1, 0);
	wattrset(win_status, color(TEXT_COLOR_BLACK, TEXT_COLOR_WHITE));
	whline(win_status, ' ', COLS);
	wnoutrefresh(win_status);
    }

    keypad(win_main, TRUE);

    // update screen
    // doupdate();

    // update configuration data
    cfg->win_main    = win_main;
    cfg->win_status  = win_status;
    cfg->win_menubar = win_menubar;

    cfg->opts = opts;
    cfg->win_popup_height = 6;
    cfg->win_popup_width  = 20;

    term_driver->cfg = cfg;
    screen_special_chars = ssc;
}

void
term_curses_close(void)
{
    endwin();
    //fflush(stdout);
}

void
term_curses_set_cursor_position(unsigned int x, unsigned int y)
{
    term_curses_cfg_t *cfg = term_driver->cfg;
    WINDOW *win_main       = cfg->win_main;

    // position cursor
    wmove(win_main, y, x);
}

void
term_curses_set_text_attribs(int fg, int bg, int attrs)
{
    term_curses_cfg_t *cfg = term_driver->cfg;
    WINDOW *win_main       = cfg->win_main;

    wattrset(win_main, color(fg, bg));

    if (attrs & TEXT_ATTRIB_DIM)
	wattron(win_main, A_DIM);
    if (attrs & TEXT_ATTRIB_BOLD)
	wattron(win_main, A_BOLD);
    if (attrs & TEXT_ATTRIB_UNDERLINE)
	wattron(win_main, A_UNDERLINE);
    if (attrs & TEXT_ATTRIB_REVERSE)
	wattron(win_main, A_REVERSE);
    if (attrs & TEXT_ATTRIB_BLINK)
	wattron(win_main, A_BLINK);
}

void
term_curses_set_char(int c)
{
    term_curses_cfg_t *cfg = term_driver->cfg;
    WINDOW *win_main       = cfg->win_main;

    waddch(win_main, (chtype)c);
}

void
term_curses_update(void)
{
    term_curses_cfg_t *cfg = term_driver->cfg;
    WINDOW *win_main       = cfg->win_main;
    WINDOW *win_menubar    = cfg->win_menubar;
    WINDOW *win_status     = cfg->win_status;

    wnoutrefresh(win_main);
    if (cfg->opts & CURSES_MENU_BAR)
	wnoutrefresh(win_menubar);
    if (cfg->opts & CURSES_STATUS_BAR)
	wnoutrefresh(win_status);

    doupdate();
}

void
term_curses_get_screen_size(unsigned int *width, unsigned int *height)
{
    term_curses_cfg_t *cfg = term_driver->cfg;

    *width  = COLS;

    if (cfg->opts & CURSES_MENU_BAR || cfg->opts & CURSES_STATUS_BAR)
	*height = cfg->win_main_height;	// manage the status bar seperately
    else
	*height = LINES;
}

void
term_curses_resize(void)
{
    struct winsize sz;
    int err;
    term_curses_cfg_t *cfg = term_driver->cfg;
    WINDOW *win_main       = cfg->win_main;
    WINDOW *win_menubar    = cfg->win_menubar;
    WINDOW *win_status     = cfg->win_status;

    err = ioctl(fileno(stdin), TIOCGWINSZ, &sz);
    if (err == -1) {
        fprintf(stderr, "cannot obtain window size: %s", strerror(errno));
	exit(1);
    }

    if (cfg->winch_height == sz.ws_row && cfg->winch_width == sz.ws_col)
	return;

    cfg->winch_height = sz.ws_row;
    cfg->winch_width  = sz.ws_col;
    resizeterm(cfg->winch_height, cfg->winch_width);

    cfg->win_main_height = LINES;
    int y = 0;
    if (cfg->opts & CURSES_MENU_BAR) {
	cfg->win_main_height--;
	y++;
    }
    if (cfg->opts & CURSES_STATUS_BAR)
	cfg->win_main_height--;

    wresize(cfg->win_main, cfg->win_main_height, COLS);
    wnoutrefresh(win_main);

    if (cfg->opts & CURSES_MENU_BAR) {
	wresize(cfg->win_menubar, 1, COLS);
	whline(win_menubar, ' ', COLS);
	wnoutrefresh(win_menubar);
    }

    if (cfg->opts & CURSES_STATUS_BAR) {
	wresize(cfg->win_status, 1, COLS);
	mvwin(cfg->win_status, LINES - 1, 0);
	whline(win_status, ' ', COLS);
	wnoutrefresh(win_status);
    }
}

void
term_curses_check_events(void)
{
    // apparently the ncurses/xterm mouse functionality doesn't work or somehow I am not using it right
    return;
#if 0
    term_curses_cfg_t *cfg = term_driver->cfg;
    MEVENT mevent;
    int err = 0;
    static int i = 0;
    int key = 0;

    memset(&mevent, 0, sizeof(MEVENT));
    if (wgetch(cfg->win_main) == KEY_MOUSE) {
	err = getmouse(&mevent);
	    if (err == OK) {
	CALL(term_driver->mouse_motion_handler)(mevent.x, mevent.y);
	//	CALL(term_driver->mouse_button_handler)();
	    }
   }
//		exit(0);
	    if (mevent.x != 0 || mevent.y != 0)
		mvwprintw(cfg->win_status, 0, 1, "[%d]: %d %d %d [%d %d] {%d}", i++, err, OK, ERR, mevent.x, mevent.y, key);
    wnoutrefresh(cfg->win_status);

    doupdate();
#endif
}

term_driver_t term_curses_driver = {
    .cfg                 = NULL,
    .open                = term_curses_open,
    .close               = term_curses_close,

    .set_cursor_position = term_curses_set_cursor_position,
    .set_text_attribs    = term_curses_set_text_attribs,
    .set_char            = term_curses_set_char,

    .get_screen_size     = term_curses_get_screen_size,

    .check_events        = term_curses_check_events,
    .update              = term_curses_update,
    .resize              = term_curses_resize,
};

#endif /* USE_CURSES */

#ifdef USE_TTY
void
term_tty_open(int opts)
{
    struct termios tty;
    int fd = fileno(stdin);

    // get current terminal attributes
    tcgetattr(fd, &tty);

    // modify terminal attributes
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    // set terminal attributes (immediately)
    tcsetattr(fd, TCSANOW, &tty);
}

void
term_tty_close(void)
{
    struct termios tty;
    int fd = fileno(stdin);

    // get current terminal attributes
    tcgetattr(fd, &tty);

    // modify terminal attributes
    tty.c_lflag |= ICANON;
    tty.c_lflag |= ECHO;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    // set terminal attributes (immediately)
    tcsetattr(fd, TCSANOW, &tty);
}

void
term_tty_set_specials(screen_special_chars_t *ssc)
{
    ssc->vline      = '|';
    ssc->hline      = '-';
    ssc->ul         = '+';
    ssc->ll         = '+';
    ssc->ur         = '+';
    ssc->lr         = '+';
    ssc->vh_cross   = '+';
    ssc->shade0     = '#';
    ssc->shade1     = '#';
    ssc->shade2     = '#';
    ssc->shade3     = '#';
    ssc->diamond    = ' ';
    ssc->bullet     = '*';
    ssc->degree     = ' ';
    ssc->plus_minus = ' ';
}

void
term_tty_set_specials_xterm(screen_special_chars_t *ssc)
{
    ssc->vline      = 120 | TEXT_ALT_CHARSET;
    ssc->hline      = 113 | TEXT_ALT_CHARSET; // or 114
    ssc->ul         = 108 | TEXT_ALT_CHARSET;
    ssc->ll         = 109 | TEXT_ALT_CHARSET;
    ssc->ur         = 107 | TEXT_ALT_CHARSET;
    ssc->lr         = 106 | TEXT_ALT_CHARSET;
    ssc->vh_cross   = 110 | TEXT_ALT_CHARSET;
    ssc->shade0     =  97 | TEXT_ALT_CHARSET;
    ssc->shade1     =  97 | TEXT_ALT_CHARSET;
    ssc->shade2     =  97 | TEXT_ALT_CHARSET;
    ssc->shade3     =  97 | TEXT_ALT_CHARSET;
    ssc->diamond    =  96 | TEXT_ALT_CHARSET;
    ssc->bullet     = 183 | TEXT_ALT_CHARSET;
    ssc->degree     = 176 | TEXT_ALT_CHARSET; // or 102
    ssc->plus_minus = 177 | TEXT_ALT_CHARSET;
}

void
term_tty_set_specials_vga(screen_special_chars_t *ssc)
{
    ssc->vline      = 179;
    ssc->hline      = 196;
    ssc->ul         = 218;
    ssc->ll         = 192;
    ssc->ur         = 191;
    ssc->lr         = 217;
    ssc->vh_cross   = 197;
    ssc->shade0     = 219;
    ssc->shade1     = 178;
    ssc->shade2     = 177;
    ssc->shade3     = 176;
    ssc->diamond    =   4;	// only works on DOS
    ssc->bullet     = 249;
    ssc->degree     = 248;
    ssc->plus_minus = 241;
}

void
term_tty_set_specials_linux_console(screen_special_chars_t *ssc)
{

}

term_driver_t term_tty_driver = {
    .open         = term_tty_open,
    .close        = term_tty_close,
};
#endif /* USE_TTY */


#ifdef USE_INT10
void
term_int10_clear(void)
{
    union REGS regs;

    // NOTE: I have not been able to find documentation
    //       for this specific call in the interuppt list
    //       document
    regs.w.cx = 0;
    regs.w.dx = 0x1850;
    regs.h.bh = 7;
    regs.w.ax = 0x0600;

    inter(0x10, &regs, &regs);
}

void
term_int10_set_cursor_position(unsigned int x, unsigned int y)
{
    term_int10_cfg_t *cfg;
    union REGS regs;

    // INTERRUP.A:2105  is the location of another set_cursor_position interrupt

    // INTERRUP.A:2212  is AH = 06h but it says its for scroll window
    //                  not sure what the deal with that is

    // DH = row  DL = column  (0 = top/left)
    regs.w.dx = (unsigned short)(( y << 8 ) + x - 0x0101);
    regs.h.bh = 7;	// page number
    regs.w.ax = 0x0600;

    inter(0x10, &regs, &regs);

    cfg->x = x;
    cfg->y = y;
}

void
term_int10_set_text_attribs(int fg, int bg, int attrs)
{
    term_int10_cfg_t *cfg;

    cfg = term_driver->cfg;
    cfg->fg_color = fg;
    cfg->bg_color = bg;
    cfg->attribs  = attrs;
}

void
term_int10_set_char(uint32_t c)
{
    uint8_t attr;
    term_int10_cfg_t *cfg;
    union REGS regs;

    cfg = term_driver->cfg;
/*
#define black   0
#define blue    1
#define green   2
#define cyan    3
#define red     4
#define magenta 5
#define yellow  6
#define white   7
*/

    // translates our colors to BIOS colors
    int color_map[] = {0,4,2,6,1,5,3,7};

/*
#define TEXT_COLOR_BLACK   0
#define TEXT_COLOR_RED     1
#define TEXT_COLOR_GREEN   2
#define TEXT_COLOR_YELLOW  3
#define TEXT_COLOR_BLUE    4
#define TEXT_COLOR_MAGENTA 5
#define TEXT_COLOR_CYAN    6
#define TEXT_COLOR_WHITE   9
#define TEXT_COLOR_DEFAULT 7
*/

// 76543210
// YBBBXFFF
// F = foreground color
// X = foreground blink/bright
// B = background color
// Y = background blink/bright

    attr = 0;

    attr |= (((color_map[cfg->fg_color] >> 0) & 1) << 0);
    attr |= (((color_map[cfg->fg_color] >> 1) & 1) << 1);
    attr |= (((color_map[cfg->fg_color] >> 2) & 1) << 2);

    attr |= (((color_map[cbg->bg_color] >> 0) & 1) << 4);
    attr |= (((color_map[cbg->bg_color] >> 1) & 1) << 5);
    attr |= (((color_map[cbg->bg_color] >> 2) & 1) << 6);

    // INTERRUP.A:2284  
    // if this is limited see: int 0x43 & 0x44 

    regs.h.ah = 0x09;
    regs.h.al = (uint8_t)c;
    regs.h.bl = (uint8_t)attr; // attribute
    regs.w.cx = 1;

    // this should work :)
    inter(0x10, &regs, &regs);
}

void
term_int10_get_screen_size(unsigned int *width, unsigned int *height)
{
    // BUG: replace this with the actual screen size
    *width  = 80;
    *height = 25;
}

term_int10_cfg_t term_int10_cfg;
term_driver_t term_int10_driver = {
    .cfg                 = &term_int10_cfg,
    .open                = term_int10_open,
    .close               = term_int10_close,

    .set_cursor_position = term_int10_set_cursor_position,
    .set_text_attribs    = term_int10_set_text_attribs,
    .set_char            = term_int10_set_char,

    .get_screen_size     = term_int10_get_screen_size,
};

#endif /* USE_INT10 */

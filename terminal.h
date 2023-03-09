#ifndef _TERMINAL_H
#define _TERMINAL_H

// text attributes
// Look at the values of A_* attributes, they are standardized and fix these if we can
#define TEXT_ATTRIB_NONE            0
#define TEXT_ATTRIB_BOLD            (1 << 1)
#define TEXT_ATTRIB_UNDERLINE       (1 << 2)
#define TEXT_ATTRIB_REVERSE         (1 << 3)
#define TEXT_ATTRIB_BLINK           (1 << 4)
#define TEXT_ATTRIB_DIM             (1 << 5)

// use these in the attribs bitmask
// use the vt100/xterm/console/etc alternate characterset
#define TEXT_ATTRIB_CHARSET_ALT     (1 << 6)
// use UTF-8 / Unicode
#define TEXT_ATTRIB_CHARSET_UNICODE (1 << 6)

// color definitions
// would like to rename these to something
// easier to type
#define TEXT_COLOR_BLACK   0
#define TEXT_COLOR_RED     1
#define TEXT_COLOR_GREEN   2
#define TEXT_COLOR_YELLOW  3
#define TEXT_COLOR_BLUE    4
#define TEXT_COLOR_MAGENTA 5
#define TEXT_COLOR_CYAN    6
#define TEXT_COLOR_WHITE   9
#define TEXT_COLOR_DEFAULT 7

// 0xE000 -> 0xF8FF = private use block
#define UNICODE_PRIVATE_OFFSET 0xE000

// use this when you want to specify a particular
// character uses the alternate console characterset
// UTF-8 only uses 31 bits, so this seems to be safe
#define TEXT_ALT_CHARSET	(1 << 31)

typedef struct {
    void  *cfg;
    void  (*open)(int opts);
    void  (*close)(void);

    void  (*get_screen_size)(unsigned int *width, unsigned int *height);
    void  (*set_cursor_position)(unsigned int x, unsigned int y);
    void  (*set_text_attribs)(int foreground, int background, int attribs);
    void  (*set_char)(int c);

    void  (*mouse_button_handler)(unsigned int num, unsigned int state);
    void  (*mouse_motion_handler)(unsigned int x, unsigned int y);
    void  (*update)(void);
    void  (*resize)(void);

    // should be called periodically to dispatch things like mouse events
    void  (*check_events)(void);
} term_driver_t;

extern term_driver_t *term_driver;

#ifdef USE_CURSES
#include <ncursesw/ncurses.h>

// these are in the ncurses lib which could be
// useful in another program but not this one
#define CURSES_MENU_BAR   (1 << 1)
#define CURSES_STATUS_BAR (1 << 2)
#define CURSES_MOUSE      (1 << 3)

typedef struct {
    WINDOW *win_main;		// main drawing area
    int win_main_height;
    WINDOW *win_status;		// status bar
    WINDOW *win_menubar;	// menu bar
    WINDOW *win_popup;		// reaction popup
    WINDOW *win_popup_shadow;	// drop shadow for popup window
    int win_popup_width;
    int win_popup_height;
    int opts;
    int winch_width;
    int winch_height;
} term_curses_cfg_t;

extern term_driver_t term_curses_driver;
#endif /* USE_CURSES */

#ifdef USE_TTY
// this driver will use raw VT100/XTerm codes for drawing
// but it is not quite complete yet
typedef struct {
    int opts;
} term_tty_cfg_t;

extern term_driver_t term_tty_driver;
#endif /* USE_TTY */

#ifdef USE_INT10
// this driver uses BIOS INT10 calls for drawing
// good for MS-DOS & small operating systems
typedef struct {
    // we don't actually change the attributes
    // immediately, we do that on the set_char 
    // call, so we save them here
    int x;
    int y;
    int fg_color;
    int bg_color;
    int attribs;
} term_int10_cfg_t;

#ifdef __386__
// 32-bit mode
#define inter(n, r1, r2) int386(n, r1, r2)
#else
// 8086 / 16-bit mode
#define inter(n, r1, r2) int86(n, r1, r2)
#endif

extern term_driver_t term_int10_driver;
#endif

#endif

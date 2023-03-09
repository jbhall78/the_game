#ifndef _WIDGET_H
#define _WIDGET_H

#include <stdint.h>

#include "slist.h"
#include "screen.h"

typedef enum {
    WIDGET_ROOT       = 0x01,
    WIDGET_WINDOW     = 0x02,

    WIDGET_TEXT_LABEL = 0x11,
    WIDGET_TEXT_INPUT = 0x12,
    WIDGET_TEXT_AREA  = 0x13,

    WIDGET_SELECT     = 0x21,

    WIDGET_BUTTON     = 0x31,
    WIDGET_CHECKBOX   = 0x32,
    WIDGET_PROGRESS   = 0x33,

    WIDGET_STATUS_BAR = 0x41,

    WIDGET_USER       = 0x99,	// user defined widget
} widget_type_t;

#define WIDGET_ATTRIB_TYPES_MAX 256
#define WIDGET_ATTRIB_NORMAL        0x01
#define WIDGET_ATTRIB_ACTIVE        0x02
#define WIDGET_ATTRIB_BORDER        0x03
#define WIDGET_ATTRIB_BORDER_ACTIVE 0x04
#define WIDGET_ATTRIB_SHADOW        0x05
#define WIDGET_ATTRIB_SHADOW_ACTIVE 0x06
#define WIDGET_ATTRIB_DEPRESSED     0x07

typedef enum {
    WIDGET_LEFT   = 0x00,
    WIDGET_CENTER = 0x01,
    WIDGET_RIGHT  = 0x02
} widget_alignment_t;

typedef struct {
    int foreground;
    int background;
    int attribs;
} widget_attribs_t;

#define widget_apply_attribs(s, w, t) \
    screen_set_text_attribs(s, \
	    (w)->attribs[t].foreground, \
	    (w)->attribs[t].background, \
	    (w)->attribs[t].attribs)

#define widget_set_attribs(w, t, f, b, a) \
    (w)->attribs[t].foreground = f; \
    (w)->attribs[t].background = b; \
    (w)->attribs[t].attribs    = a;
typedef struct widget_s {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    widget_type_t type;

    uint8_t visible;
    uint8_t can_focus;

    uint32_t n_children;
    slist_t *children;
    struct widget_s *parent;
    void *data;

    void (*draw)(screen_t *s, struct widget_s *w);
    void (*key)(struct widget_s *w, int key, int type);

    widget_attribs_t attribs[WIDGET_ATTRIB_TYPES_MAX];
} widget_t;

typedef struct {
    widget_t *active;
} widget_root_t;

/* window is just a virtual container widget which can be larger then the screen and where clipping is performed */
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t x;
    uint32_t y;

    widget_t *active;

    // we could add some decorations and stuff to this later i guess
} widget_window_t;

typedef struct {
    char *txt;
    widget_alignment_t alignment;
} widget_text_label_t;

typedef struct {
    char *txt;
} widget_status_bar_t;

typedef struct {
    char buf[256];
    uint16_t x;
    uint16_t max_length;
    int password;	// display '*'s instead of the text entered
    widget_alignment_t alignment;
} widget_text_input_t;

// auto resize in x or y direction by this many characters
#define WIDGET_TEXT_AREA_RESIZE_INCREMENTS 128
typedef struct {
    screen_char_t *buf;	// where text contents are stored

    // width / height of text in the area, can be automatically resized
    uint32_t width;
    uint32_t height;

    // limits on auto resizing
    uint32_t width_max;
    uint32_t height_max;

    // set to 1 if you want this widget to resize itself automatically when
    // the user enters text
    uint32_t resizeable;

    // scroll position
    uint32_t scrolling_enabled;
    uint32_t scroll_x;
    uint32_t scroll_y;

    // cursor position
    uint32_t x;
    uint32_t y;
} widget_text_area_t;

typedef struct {
    int checked;
} widget_checkbox_t;

typedef struct {
    double min;	// lower boundary
    double max; // upper boundary
    double cur; // current position
} widget_progress_t;

typedef struct {
    slist_t *options;
    uint32_t selected;
} widget_select_t;

typedef struct {
    char *label;
    widget_alignment_t alignment;
    int depressed;
    void *handler_data;
    void (*handler)(widget_t *w, void *data);
} widget_button_t;

widget_t *widget_new(widget_t *parent,
	widget_type_t type,
	uint32_t width, uint32_t height, uint32_t x, uint32_t y);
void widget_text_label_set_attribs(widget_t *w,
	int foreground, int background, int attribs);
void widget_text_label_set_text(widget_t *w, char *txt);
void widget_text_label_set_alignment(widget_t *w,
	widget_alignment_t alignment);
int widget_text_input_set_max_length(widget_t *w, int max_length);
char * widget_text_input_get_text(widget_t *w);
void widget_key(widget_t *root, int key, int type);
void widget_draw(screen_t *s, widget_t *w);

void widget_text_input_set_attribs(widget_t *w,
	int foreground, int background, int attribs,
	int foreground_active, int background_active, int attribs_active);
void widget_text_input_set_alignment(widget_t *w,
	widget_alignment_t alignment);

int widget_is_active(widget_t *w);

void widget_button_set_label(widget_t *w, char *str);
void widget_button_set_attribs(widget_t *w,
	int, int, int,   // normal
	int, int, int,   // active
	int, int, int,   // border
	int, int, int,   // border active
	int, int, int);  // drop shadow

void widget_button_set_label(widget_t *w, char *label);
void widget_status_bar_set_text(widget_t *w, char *txt);

#define widget_button_set_depressed(w, x) \
    ((widget_button_t *)w->data)->depressed = x;

#define widget_checkbox_set_checked(w, x) \
    ((widget_checkbox_t *)w->data)->checked = x;

#define widget_checkbox_toggle_checked(w) \
    ((widget_checkbox_t *)w->data)->checked ^= 1;

#define widget_button_set_handler(w, h, d) \
    ((widget_button_t *)w->data)->handler = h; \
    ((widget_button_t *)w->data)->handler_data = d;

// we may want to make all widgets like user defined ones
#define widget_user_set_handlers(w, _draw, _key) \
    w->draw = _draw; \
    w->key  = _key;	

#define widget_set_can_focus(w, f) \
    (w)->can_focus = f;



#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>

#include "widget.h"
#include "screen.h"
#include "terminal.h"
#include "dbg.h"
#include "input.h"

widget_t *
widget_new(
	widget_t *parent,
	widget_type_t type,
	uint32_t width,
	uint32_t height,
	uint32_t x,
	uint32_t y)
{
    widget_t *widget;
    widget_root_t       *widget_root;
    widget_window_t     *widget_window;

/*
    widget_text_input_t *widget_text_input;
    widget_text_label_t *widget_text_label;
    widget_text_area_t  *widget_text_area;
    widget_select_t     *widget_select;
    widget_button_t     *widget_button;
    widget_checkbox_t   *widget_checkbox;
    widget_progress_t   *widget_progress;
*/
    uint32_t data_size;

    widget = malloc(sizeof(widget_t));
    assert(widget != NULL);
    memset(widget, 0, sizeof(widget_t));

    widget->parent  = parent;
    widget->type    = type;
    widget->width   = width;
    widget->height  = height;
    widget->x       = x;
    widget->y       = y;
    widget->visible = 1;

    switch (type) {
	case WIDGET_ROOT:
	    assert(widget->parent == NULL);
	    data_size = sizeof(widget_root_t);
	    widget->data = malloc(data_size);
	    memset(widget->data, 0, data_size);

	    break;
	case WIDGET_WINDOW:
	    assert(widget->parent->type == WIDGET_ROOT);
	    data_size = sizeof(widget_window_t);
	    widget->data = malloc(data_size);
	    memset(widget->data, 0, data_size);

	    widget_root = (widget_root_t *)widget->parent->data;
	    //if (widget_root->active == NULL)
	    widget_root->active = widget;

	    widget_set_attribs(widget, WIDGET_ATTRIB_NORMAL,
		    TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, 0);
	    break;

	case WIDGET_USER:
	    data_size = sizeof(widget_root_t);
	    widget->data = NULL;

	    widget_set_attribs(widget, WIDGET_ATTRIB_NORMAL,
		    TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, 0);
	    break;
	case WIDGET_TEXT_LABEL:
	    data_size = sizeof(widget_text_label_t);
	    widget->data = malloc(data_size);
	    memset(widget->data, 0, data_size);

	    widget_set_attribs(widget, WIDGET_ATTRIB_NORMAL,
		    TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, 0);

	    break;
	case WIDGET_STATUS_BAR:
	    data_size = sizeof(widget_status_bar_t);
	    widget->data = malloc(data_size);
	    memset(widget->data, 0, data_size);

	    widget_set_attribs(widget, WIDGET_ATTRIB_NORMAL,
		    TEXT_COLOR_BLACK, TEXT_COLOR_WHITE, 0);

	    break;
	case WIDGET_TEXT_INPUT:
	    data_size = sizeof(widget_text_input_t);
	    widget->data = malloc(data_size);
	    memset(widget->data, 0, data_size);

	    widget->can_focus = 1;
	    widget_text_input_set_max_length(widget, -1);
	    //dbg("widget_text_input_set_max_length = %d\n", l);

	    widget_set_attribs(widget, WIDGET_ATTRIB_ACTIVE,
		    TEXT_COLOR_BLACK, TEXT_COLOR_WHITE, 0);

	    widget_set_attribs(widget, WIDGET_ATTRIB_NORMAL,
		    TEXT_COLOR_BLACK, TEXT_COLOR_GREEN, 0);
	    break;
	case WIDGET_TEXT_AREA:
	    data_size = sizeof(widget_text_area_t);
	    widget->data = malloc(data_size);
	    memset(widget->data, 0, data_size);

	    widget->can_focus = 1;
	    break;
	case WIDGET_SELECT:
	    data_size = sizeof(widget_select_t);
	    widget->data = malloc(data_size);
	    memset(widget->data, 0, data_size);

	    widget->can_focus = 1;
	    break;
	case WIDGET_BUTTON:
	    data_size = sizeof(widget_button_t);
	    widget->data = malloc(data_size);
	    memset(widget->data, 0, data_size);

	    widget->can_focus = 1;

	    widget_set_attribs(widget, WIDGET_ATTRIB_NORMAL,
		    TEXT_COLOR_WHITE, TEXT_COLOR_RED, 0);
	    widget_set_attribs(widget, WIDGET_ATTRIB_BORDER,
		    TEXT_COLOR_WHITE, TEXT_COLOR_RED, 0);
	    widget_set_attribs(widget, WIDGET_ATTRIB_DEPRESSED,
		    TEXT_COLOR_WHITE, TEXT_COLOR_RED, 0);
	    widget_set_attribs(widget, WIDGET_ATTRIB_ACTIVE,
		    TEXT_COLOR_YELLOW, TEXT_COLOR_GREEN, 0);
	    widget_set_attribs(widget, WIDGET_ATTRIB_BORDER_ACTIVE,
		    TEXT_COLOR_YELLOW, TEXT_COLOR_GREEN, 0);
	    widget_set_attribs(widget, WIDGET_ATTRIB_SHADOW,
		    TEXT_COLOR_WHITE, TEXT_COLOR_BLACK, 0);
	    widget_set_attribs(widget, WIDGET_ATTRIB_SHADOW_ACTIVE,
		    TEXT_COLOR_WHITE, TEXT_COLOR_BLACK, 0);
	    break;
	case WIDGET_CHECKBOX:
	    data_size = sizeof(widget_checkbox_t);
	    widget->data = malloc(data_size);
	    memset(widget->data, 0, data_size);


	    widget->can_focus = 1;

	    widget_set_attribs(widget, WIDGET_ATTRIB_NORMAL,
		    TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, 0);

	    widget_set_attribs(widget, WIDGET_ATTRIB_ACTIVE,
		    TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, 0);
	    break;
	case WIDGET_PROGRESS:
	    data_size = sizeof(widget_progress_t);
	    widget->data = malloc(data_size);
	    memset(widget->data, 0, data_size);

	    break;
	default:
	    abort();
	    break;
    }

    if (parent != NULL) {
	slist_t *item = slist_new();
	item->data = widget;
	widget->parent->children = slist_append(widget->parent->children,
		                                item);
	widget->parent->n_children++;

	if (widget->can_focus && parent->type == WIDGET_WINDOW) {
	    widget_window = (widget_window_t *)widget->parent->data;
	    //if (widget_window->active == NULL)
		widget_window->active = widget;
	}
    }

    return widget;
}

/* whenever I can get the mouse working ... */

/*
void
widget_mmotion()
{

}
*/

/*
void
widget_mbutton()
{

}
*/

void
widget_destroy(widget_t *widget)
{
    free(widget->data);
    free(widget);
}

// returns the viewport/onscreen position of the widget based on the state
// of parent widgets
// TODO: right now we are only concerning ourselves with the offsets,
// fix width and height later
// NOTE: I could do this recursively, but it gets a little verbose with
// all the arguments which would need to be passed along
static void
widget_calculate_relativity(
	widget_t *widget,
 	uint32_t *width, uint32_t *height,
	uint32_t *x, uint32_t *y)
{
    slist_t *lineage = NULL;
    slist_t *item = NULL;
    widget_t *w;
    uint32_t xx, yy;

    assert(widget != NULL);

    // work our way torwards root,
    // building a list of pointers to all of the nodes along the way
    for (w = widget->parent; w; w = w->parent) {
	item = slist_new();
	item->data = w;
	lineage = slist_add_start(lineage, item);
    }

    w = (widget_t *)lineage->data;
    assert(w->type == WIDGET_ROOT);

    // compute the offset
    xx = yy = 0;
    for (item = lineage; item; item = item->next) {
	w = (widget_t *)item->data;
	xx += w->x;
	yy += w->y;
	//dbg("widget[0x%02x] offset: [%d,%d]\n", w->type, xx, yy);
    }

    slist_destroy(lineage);

    if (x != NULL)
	*x = xx;
    if (y != NULL)
	*y = yy;
}

static void
widget_window_draw(screen_t *s, widget_t *w)
{
    int i, j;
    uint32_t rx, ry, x, y;

    assert(w != NULL);
    assert(w->data != NULL);

    widget_calculate_relativity(w, NULL, NULL, &rx, &ry);

    x = rx + w->x;
    y = ry + w->y;

    widget_apply_attribs(s, w, WIDGET_ATTRIB_NORMAL);

    for (j = 0; j < w->height; j++) {
	for (i = 0; i < w->width; i++) {
	    screen_set_char(s, x+i, y+j, ' ');
	}
    }
}

static void
widget_text_label_draw(screen_t *s, widget_t *w)
{
    uint32_t rx, ry, x, y;
    widget_text_label_t *label;

    assert(w != NULL);
    assert(w->data != NULL);

    label = w->data;

    widget_calculate_relativity(w, NULL, NULL, &rx, &ry);

    x = rx + w->x;
    y = ry + w->y;

    if (label->txt != NULL) {
	widget_apply_attribs(s, w, WIDGET_ATTRIB_NORMAL);
	screen_printf(s, x, y, "%s", label->txt);
    }
}

static void
widget_status_bar_draw(screen_t *s, widget_t *w)
{
    widget_status_bar_t *status_bar;
    uint32_t rx, ry, x, y, i;

    assert(w != NULL);
    assert(w->data != NULL);

    status_bar = (widget_status_bar_t *)w->data;

    widget_calculate_relativity(w, NULL, NULL, &rx, &ry);

    x = rx + w->x;
    y = ry + w->y;

    widget_apply_attribs(s, w, WIDGET_ATTRIB_NORMAL);
    // clear entire area to background color
    for (i = x; i < w->width; i++)
	screen_set_char(s, i, y, ' ');

    if (status_bar->txt != NULL) {
	screen_printf(s, x+1, y, "%s", status_bar->txt);
    }
}
static void
widget_text_input_draw(screen_t *s, widget_t *w)
{
    widget_text_input_t *input;
    uint32_t rx, ry, x, y;
    uint32_t len;
    uint32_t i;
    int is_active;

    assert(w != NULL);
    assert(w->data != NULL);

    input = (widget_text_input_t *)w->data;
    is_active = widget_is_active(w);

    widget_calculate_relativity(w, NULL, NULL, &rx, &ry);

    x = rx + w->x;
    y = ry + w->y;

    len = strlen(input->buf);

    if (is_active)
	widget_apply_attribs(s, w, WIDGET_ATTRIB_ACTIVE);
    else
	widget_apply_attribs(s, w, WIDGET_ATTRIB_NORMAL);

    // draw background
    for (i = 0; i < w->width; i++) {
	screen_set_char(s, x + i, y, ' ');
    }

    if (len > 0) {
	//dbg("print: [%d, %d] %s", x, y, input->buf);
	// draw input buffer
	for (i = 0; i < w->width && i < len; i++) {
	    screen_set_char(s, x + i, y, input->buf[i]);
	}
	//screen_printf(s, x, y, "%-*s", w->width, input->buf);
    }

    if (is_active) {
	int px = (len >= w->width) ? x + w->width - 1 : x + len;
	screen_set_text_cursor(s, px, y);
    }
}

static void
widget_button_draw(screen_t *s, widget_t *w)
{
    widget_button_t *button;
    uint32_t rx, ry, x, y;
    uint32_t i, j;
    int is_active;
    int len;
    screen_special_chars_t *ssc;

    assert(w != NULL);
    assert(w->data != NULL);

    button = (widget_button_t *)w->data;
    is_active = widget_is_active(w);
    len = strlen(button->label);
    ssc = screen_special_chars;

    if (w->visible != 1)
	return;

    widget_calculate_relativity(w, NULL, NULL, &rx, &ry);

    x = rx + w->x;
    y = ry + w->y;

    if (button->depressed) {
	widget_apply_attribs(s, w, WIDGET_ATTRIB_DEPRESSED);

	x++;
	y++;
    } else {
	// draw drop shadow
	if (is_active)
	    widget_apply_attribs(s, w, WIDGET_ATTRIB_SHADOW_ACTIVE);
	else
	    widget_apply_attribs(s, w, WIDGET_ATTRIB_SHADOW);

	for (j = 0; j < w->height; j++) {
	    for (i = 0; i < w->width; i++) {
		screen_set_char(s, x+i+1, y+j+1, ' ');
	    }
	}
    }

    if (is_active) {
	widget_apply_attribs(s, w, WIDGET_ATTRIB_ACTIVE);
    } else {
	widget_apply_attribs(s, w, WIDGET_ATTRIB_NORMAL);
    }	
    for (j = 0; j < w->height; j++) {
       	for (i = 0; i < w->width; i++) {
	    screen_set_char(s, x+i, y+j, ' ');
	}
    }

    if (button->label != NULL) {
	j = w->height / 2;
	i = w->width / 2 - (len / 2);
	screen_printf(s, x + i, y + j, "%s", button->label);
    }

    // draw border
    if (is_active) {
	widget_apply_attribs(s, w, WIDGET_ATTRIB_BORDER_ACTIVE);
    } else {
	widget_apply_attribs(s, w, WIDGET_ATTRIB_BORDER);
    }
    if (w->height >= 3 && w->width >= (len+2)) {
	// upper left -> upper right
	screen_set_char(s, x, y, ssc->ul);
	for (i = 1; i < w->width-1; i++)
	    screen_set_char(s, x+i, y, ssc->hline);
	screen_set_char(s, x+w->width-1, y, ssc->ur);

	// vertical lines on both sides
	for (j = 1; j < w->height; j++) {
	    screen_set_char(s, x, y+j, ssc->vline);
	    screen_set_char(s, x+w->width-1, y+j, ssc->vline);
	}

	// bottom left -> bottom right
	screen_set_char(s, x, y+w->height-1, ssc->ll);
	for (i = 1; i < w->width-1; i++)
	    screen_set_char(s, x+i, y+w->height-1, ssc->hline);
	screen_set_char(s, x+w->width-1, y+w->height-1, ssc->lr);
    }

}

static void
widget_checkbox_draw(screen_t *s, widget_t *w)
{
    widget_checkbox_t *checkbox;
    uint32_t rx, ry, x, y;
    uint32_t i = 0;
    int is_active;

    assert(w != NULL);
    assert(w->data != NULL);

    checkbox = (widget_checkbox_t *)w->data;
    is_active = widget_is_active(w);

    widget_calculate_relativity(w, NULL, NULL, &rx, &ry);

    x = rx + w->x;
    y = ry + w->y;

    if (is_active)
	widget_apply_attribs(s, w, WIDGET_ATTRIB_ACTIVE);
    else
	widget_apply_attribs(s, w, WIDGET_ATTRIB_NORMAL);

    // draw border
    screen_set_char(s, x + i, y, '[');
    i++;

    if (checkbox->checked)
	screen_set_char(s, x + i, y, '*');
    else
	screen_set_char(s, x + i, y, ' ');
    i++;

    screen_set_char(s, x + i, y, ']');
    i++;

    if (is_active) {
	int px = x + 1;
	screen_set_text_cursor(s, px, y);
    }

    dbg("checkbox drawn: [%c]\n", (checkbox->checked) ? '*' : ' ');
}

void
widget_draw(screen_t *s, widget_t *w)
{
    slist_t *item;

    assert(w != NULL);

    // draw this widget
    switch (w->type) {
	case WIDGET_ROOT:
	    break;
	case WIDGET_WINDOW:
	    widget_window_draw(s, w);
	    break;
	case WIDGET_TEXT_LABEL:
	    widget_text_label_draw(s, w);
	    break;
	case WIDGET_TEXT_INPUT:
	    widget_text_input_draw(s, w);
	    break;
	case WIDGET_BUTTON:
	    widget_button_draw(s, w);
	    break;
	case WIDGET_CHECKBOX:
	    widget_checkbox_draw(s, w);
	    break;
	case WIDGET_STATUS_BAR:
	    widget_status_bar_draw(s, w);
	    break;
	case WIDGET_USER:
	    CALL(w->draw)(s, w);
	    break;
	default:
	    break;
    }

    // draw children
    for (item = w->children; item; item = item->next)
	widget_draw(s, (widget_t *)item->data);
}

widget_t *
widget_get_root(widget_t *w)
{
    widget_t *p;

    assert(w != NULL);

    /* find the widget with no parents */
    for (p = w; p->parent; p = p->parent)
        ;

    /* this should be a root widget */
    if (p->type == WIDGET_ROOT)
        return p;
    else
        return NULL;
}

void
widget_text_label_set_text(widget_t *w, char *txt)
{
    widget_text_label_t *label;

    assert(w != NULL);
    assert(w->data != NULL);

    label = (widget_text_label_t *)w->data;

    if (label->txt != NULL)
	free(label->txt);

    if (txt != NULL)
	label->txt = strdup(txt);
}

void
widget_status_bar_set_text(widget_t *w, char *txt)
{
    widget_status_bar_t *status_bar;

    assert(w != NULL);
    assert(w->data != NULL);

    status_bar = (widget_status_bar_t *)w->data;

    if (status_bar->txt != NULL)
	free(status_bar->txt);

    if (txt != NULL)
	status_bar->txt = strdup(txt);
}

void
widget_text_label_set_alignment(widget_t *w, widget_alignment_t alignment)
{
    widget_text_label_t *label;

    assert(w != NULL);
    assert(w->data != NULL);

    label = (widget_text_label_t *)w->data;

    label->alignment = alignment;
}

void
widget_text_input_set_alignment(widget_t *w, widget_alignment_t alignment)
{
    widget_text_input_t *input;

    assert(w != NULL);
    assert(w->data != NULL);

    input = (widget_text_input_t *)w->data;

    input->alignment = alignment;
}

char *
widget_text_input_get_text(widget_t *w)
{
    widget_text_input_t *input;

    assert(w != NULL);
    assert(w->data != NULL);

    input = (widget_text_input_t *)w->data;

    return input->buf;
}

void
widget_select_key(widget_t *w, int key, int type)
{
    widget_select_t *select;

    assert(w != NULL);
    assert(w->data != NULL);

    select = w->data;
}

void
widget_button_key(widget_t *w, int key, int type)
{
    widget_button_t *button;

    assert(w != NULL);
    assert(w->data != NULL);

    button = w->data;

    if ((key == ' ' || key == '\r') && type == INPUT_KEY_DOWN)
	widget_button_set_depressed(w, 1);

    if ((key == ' ' || key == '\r') && type == INPUT_KEY_UP) {
	// add in an extra delay for the tty input driver
	// so the animation is visible, since the tty driver
	// generates fake KEY_UP events
	// we can't hide this in the input driver layer or else
	// we end up with delays everywhere
	if (input_driver == &input_tty_driver)
	    usleep(200 * 1000);
	widget_button_set_depressed(w, 0);
    }

    CALL(button->handler)(w, button->handler_data);
}

void
widget_text_input_key(widget_t *w, int key, int type)
{
    widget_text_input_t *input;
    int len;

    assert(w != NULL);
    assert(w->data != NULL);

    input = (widget_text_input_t *)w->data;

    len = strlen(input->buf);

    if (type != INPUT_KEY_UP)
	return;

    switch (key) {
	// backspace & delete
	case 0x08:
	case 0x7F:
	    if (len > 0)
		input->buf[len-1] = 0;
	    break;

	default:
	    if (isprint(key) && len <= input->max_length) {
		input->buf[len]   = (char)key;
		input->buf[len+1] = 0;
	    }
    }
//    dbg("widget_key: 0x%02x '%c' [%d]\n", key, key, input->max_length);

}

void
widget_key(widget_t *root, int key, int type)
{
    widget_t *active;
    widget_root_t *widget_root;

    widget_t *window;
    widget_window_t *widget_window;

    assert(root != NULL);
    assert(root->type == WIDGET_ROOT); // possibly allow WIDGET_WINDOW too

    // get the root pointers
    widget_root = (widget_root_t *)root->data;
    assert(widget_root->active != NULL); // active window

    // get the window pointers
    window = (widget_t *)widget_root->active;
    widget_window = (widget_window_t *)window->data;

    active = widget_window->active;

    // no active field to accept input
    if (active == NULL)
	return;

    // process tab key
    if (key == '\t' && type == INPUT_KEY_DOWN) {
	// focus next widget
	uint32_t i, j;

	widget_t **array = (widget_t **)slist_array(window->children);
	for (i = 0; array[i] != NULL; i++) {
	    if (array[i] == active) {
		j = i;
		// WARNING: could possibly cause an infinite loop
		//          if for some reason there can_focus is set to 0
		//          on the widget that currently has focus
		//          and no other widgets have can_focus set
		for (;;) {
		    j++;
		    //dbg("loop: 0x%08x [%d]\n", array[j], j);
		    if (array[j] == NULL) {
			j = 0;
		    }
		    if (array[j]->can_focus) {
			widget_window->active = array[j];
			free(array);
			break;
		    }
		}
		break;
	    }
	}
	return;
    } else {
       	// send keystroke to active widget for further processing
	switch (active->type) {
	    case WIDGET_TEXT_INPUT:
		widget_text_input_key(active, key, type);
		break;
/*
	    case WIDGET_TEXT_AREA:
		widget_text_area_key(active, key, type);
		break;
*/
	    case WIDGET_SELECT:
		widget_select_key(active, key, type);
		break;
	    case WIDGET_BUTTON:
		widget_button_key(active, key, type);
		break;
	    case WIDGET_CHECKBOX:
		if ((key == ' ' || key == '\r') && type == INPUT_KEY_DOWN) {
		    dbg("toggling checkbox\n");
		    widget_checkbox_toggle_checked(active);
		}
		break;
	    case WIDGET_USER:
		CALL(active->key)(active, key, type);
		break;
	    default:
		break;
	}
    }
}

int
widget_is_active(widget_t *widget)
{
    widget_t *w;
    widget_window_t *win;

    assert(widget != NULL);
    assert(widget->parent != NULL);

    for (w = widget->parent; w; w = w->parent) {
	if (w->type == WIDGET_WINDOW)
	    break;
    }

    win = (widget_window_t *)w->data;

    if (widget == win->active)
	return 1;

    return 0;
}

/* params: max_length == -1  = set to maximum allowable length */
int
widget_text_input_set_max_length(widget_t *w, int max_length)
{
    widget_text_input_t *input = (widget_text_input_t *)w->data;
    int max_allowable_length = sizeof(input->buf)-1;

    if (max_length == -1)
	input->max_length = max_allowable_length;
    else if (max_length < 0)	// invalid
	return input->max_length;
    else if (max_length > max_allowable_length) // invalid
     	return input->max_length;
    else
	input->max_length = max_length;

    return input->max_length;
}

void
widget_button_set_label(widget_t *w, char *label)
{
    widget_button_t *button = (widget_button_t *)w->data;

    button->label = strdup(label);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "game.h"
#include "screen.h"
#include "terminal.h"
#include "dbg.h"

screen_special_chars_t *screen_special_chars;

/* NOTE: requires terminal driver to be initialized */
screen_t *
screen_init(void)
{
    screen_t *screen;

    // allocate memory
    screen = malloc(sizeof(screen_t));
    assert(screen != NULL);
    memset(screen, 0, sizeof(screen_t));

    // set defaults
    screen->foreground = TEXT_COLOR_WHITE;
    screen->background = TEXT_COLOR_BLACK;
    screen->attribs    = TEXT_ATTRIB_NONE;

    screen_resize(screen);
    screen_clear(screen);

    // initialize the back buffer to the contents of the front one
    memcpy(screen->prev, screen->data,
	    screen->width * screen->height * sizeof(screen_char_t));

    return screen;
}

void
screen_clear(screen_t *screen)
{
    int x, y;

    for (y = 0; y < screen->height; y++) {
	for (x = 0; x < screen->width; x++) {
	    screen_set_char(screen, x, y, ' ');
	}
    }
}

void
screen_set_text_attribs(
	screen_t *screen,
	int foreground,
	int background,
	int attribs)
{
    assert(screen != NULL);
    screen->foreground = foreground;
    screen->background = background;
    screen->attribs    = attribs;
}

void
screen_set_char(screen_t *screen, unsigned int x, unsigned int y, uint32_t c)
{
    screen_char_t *sc;

    assert(screen != NULL);
    assert(screen->data != NULL);

    if (x >= screen->width)
	return;
    if (y >= screen->height)
	return;

    sc = screen->data + (y * screen->width) + x;

    sc->foreground = screen->foreground;
    sc->background = screen->background;
    sc->attribs    = screen->attribs;
    sc->c          = c;
}

void
screen_printf(
	screen_t *screen,
	unsigned int x,
	unsigned int y,
	char *fmt, ...)
{
    /* Guess we need no more than 100 bytes. */
    int n, size = 100;
    char *p, *str = NULL;
    va_list ap;
    unsigned int i, len;
    int xx;
    int yy;

    if ((p = malloc (size)) == NULL)
        return;

    for (;;) {
        /* Try to print in the allocated space. */
        va_start(ap, fmt);
        n = vsnprintf (p, size, fmt, ap);
        va_end(ap);

        /* If that worked, return the string. */
        if (n > -1 && n < size) {
	    str = p;
	    break;
	}

        /* Else try again with more space. */
        if (n > -1)             // glibc 2.1
            size = n+1;         // precisely what is needed
        else                    // glibc 2.0
            size *= 2;          // twice the old size

        if ((p = realloc (p, size)) == NULL)
            return;
    }

    if (str == NULL)
	return;

    len = strlen(str);
    xx = x;
    yy = y;
    for (i = 0; i < len; i++) {
	if (str[i] == '\n') {
	    yy++;
	    xx = x;
	    continue;
	}
	screen_set_char(screen, xx, yy, str[i]);
	xx++;
    }
}

void
screen_blit(
	screen_t *screen,
	anim_t *anim,
	int frame,
	unsigned int dest_x,
	unsigned int dest_y)
{
    slist_t *item;
    anim_frame_t *af;
    int i, j, x, y;
    char c;

    assert(frame < anim->n_frames);
    item = slist_index(anim->frames, frame);
    af = (anim_frame_t *)item->data;

    for (j = 0; j < af->height; j++) {
	for (i = 0; i < af->width; i++) {
	    // determine character to render
	    // TODO: expand af->data to include per char attribs
	    c = *(af->data + ((j * af->width) + i));

	    // TODO: render spinner
	    if (c == anim->char_spinner)
		continue;

	    if (c == anim->char_alpha)
		continue;

	    // translate coords
	    x = dest_x + i;
	    y = dest_y + j;

	    // cull
	    if (! (x >= 0))
		continue;

	    if (! (x < screen->width))
		continue;

	    if (! (y >= 0))
		continue;

	    if (! (y < screen->height))
		continue;

	    // draw character
	    screen_set_char(screen, x, y, c);
	}
    }
}

void
screen_set_text_cursor(screen_t *screen, int x, int y)
{
    screen->text_cursor_x = x;
    screen->text_cursor_y = y;
}

void
screen_flip(screen_t *screen)
{
    unsigned int x, y, off;
    screen_char_t *c1, *c2;

    dbg("flipping screen\n");

    // loop over each screen character,
    // if it is different from the previous one,
    // move cursor to that location and redraw
    for (y = 0; y < screen->height; y++) {
	for (x = 0; x < screen->width; x++) {
	    int update_attribs = 0;

	    // calculate offset from base address for this character
	    off = (y * screen->width) + x;
	    c1 = screen->data + off;
	    c2 = screen->prev + off;

	    // check to see if attributes need to be updated
	    if (c1->foreground != c2->foreground)
		update_attribs = 1;

	    if (c1->background != c2->background)
		update_attribs = 1;

	    if (c1->attribs != c2->attribs)
		update_attribs = 1;

	    // if either attributes or the character have changed draw it
	    if (update_attribs || c1->c != c2->c || screen->force_update) {
		term_driver->set_cursor_position(x, y);
		term_driver->set_text_attribs(
			c1->foreground, c1->background, c1->attribs);
		term_driver->set_char(c1->c);
	    }
	}
    }

    term_driver->set_cursor_position(
	    screen->text_cursor_x,
	    screen->text_cursor_y);

    // copy screen buffer to previous one so we can start drawing again
    memcpy(screen->prev, screen->data,
	    screen->width * screen->height * sizeof(screen_char_t));

    term_driver->update();
}

void
screen_resize(screen_t *screen)
{
    unsigned int width, height, size;
    screen_char_t *data, *prev;

    term_driver->get_screen_size(&width, &height);

    size = width * height * sizeof(screen_char_t);

    data = malloc(size);
    memset(data, 0, size);
    if (screen->data != NULL) {
	// TODO: copy data to new buffers
	free(screen->data);
    }

    prev = malloc(size);
    memset(data, 0, size);
    if (screen->prev != NULL) {
	// TODO: copy data to new buffers
	free(screen->prev);
    }

    screen->width  = width;
    screen->height = height;
    screen->data   = data;
    screen->prev   = prev;
}

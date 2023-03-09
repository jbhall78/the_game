#ifndef _SCREEN_H
#define _SCREEN_H

#include <stdint.h>

#include "game.h"

typedef struct {
    int foreground;
    int background;
    int attribs;
    uint32_t c;		// ASCII byte or Unicode (set attribute to enable)
} screen_char_t;

typedef struct {
    unsigned int width, height;
    screen_char_t *data;	// the buffer the user draws to
    screen_char_t *prev;	// what is currently on the screen

    int foreground;
    int background;
    int attribs;

    // text cursor position
    int text_cursor_x;
    int text_cursor_y;

    int force_update;
} screen_t;

// this structure exists so it can be changed at run time
typedef struct {
    // line drawing
    uint32_t vline;
    uint32_t hline;
    uint32_t ul;
    uint32_t ll;
    uint32_t ur;
    uint32_t lr;
    uint32_t vh_cross;

    // shading
    uint32_t shade0;
    uint32_t shade1;
    uint32_t shade2;
    uint32_t shade3;

    // characters
    uint32_t diamond;
    uint32_t bullet;
    uint32_t degree;
    uint32_t plus_minus;
} screen_special_chars_t;

extern screen_special_chars_t *screen_special_chars;

screen_t *screen_init(void);
void screen_clear(screen_t *screen);
void screen_set_char(screen_t *screen,
	unsigned int x, unsigned int y, uint32_t c);
void screen_set_text_attribs(screen_t *screen,
	int foreground, int background, int attribs);
void screen_blit(screen_t *screen,
	anim_t *anim, int frame, unsigned int dest_x, unsigned int dest_y);
void screen_flip(screen_t *screen);
void screen_resize(screen_t *screen);
void screen_printf(screen_t *screen,
	unsigned int x, unsigned int y, char *fmt, ...);
void screen_set_text_cursor(screen_t *screen, int x, int y);


#endif

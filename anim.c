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

#include "game.h"
#include "anim.h"

anim_t *
anim_load(char *filename)
{
    int fd;
    struct stat st;
    int err, n;
    char *file;
    char *buf;
    int buf_size;
    anim_t *anim;
    unsigned int line, frame = 0, x = 0, y = 0, width = 0, height = 0;
    unsigned int i, j, k;
    unsigned int len;
    char char_seperator;

    enum {
	STATE_HEADER    = 0,
	STATE_SEPERATOR = 1,
	STATE_FRAME     = 2
    } state;
    enum {
	COL_NAME       = 0,
	COL_NUM_FRAMES = 1,
	COL_SPECIALS   = 2
    } column;
    enum {
	CHAR_SEPERATOR = 0,
	CHAR_ALPHA     = 1,
	CHAR_SPINNER   = 2
    } special;

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
	fprintf(stderr,
		"ERROR: unable to load file %s: %s\n",
		filename, strerror(errno));
	exit(1);
    }

    err = stat(filename, &st);
    if (err == -1) {
	fprintf(stderr,
		"ERROR: unable to stat file %s: %s\n",
		filename, strerror(errno));
	exit(1);
    }

    // allocate space to store file contents
    buf_size = st.st_size + 1;
    file = malloc(buf_size);
    assert(file != NULL);

    // read file into buffer
    n = read(fd, file, buf_size - 1);
    file[buf_size] = 0;

    // allocate a secondary buffer
    buf = malloc(buf_size);
    assert(buf != NULL);
    memset(buf, 0, buf_size);

    // allocate animation object
    anim = malloc(sizeof(anim_t));
    assert(anim != NULL);
    memset(anim, 0, sizeof(anim_t));

    // use a finite state machine to process file contents
    line    = 1;
    frame   = -1;
    state   = STATE_HEADER;
    column  = COL_NAME;
    special = CHAR_SEPERATOR;
    j = 0;
    for (i = 0; i < st.st_size; i++) {
	// bad data / end of file
	if (file[i] == '\0')
	    break;

	// disregard these characters if we are on dos/windows
	if (file[i] == '\r')
	    continue;

	if (file[i] == '\n') {
	    line++;
	    // we need to pass \n chars down to the data frame handler
	    // so that we can determine the width of the text
	    if (state != STATE_FRAME)
		continue;
	}

	if (state == STATE_HEADER) {
	    if (line > 1)
		state++;

	    if (file[i] == '|') {
		// save buffers
		if (column == COL_NAME) {
		    len = strlen(buf);
		    if (len > 0)
			anim->name = strdup(buf);
		    j = 0;
		    memset(buf, 0, sizeof(buf));
		} else if (column == COL_NUM_FRAMES) {
		    ;
		} else if (column == COL_SPECIALS) {
		    ;
		}
		column++;
	    } else {
		// fill buffers
		if (column == COL_NAME) {
		    buf[j++] = file[i];
		} else if (column == COL_NUM_FRAMES) {
		    ;
		} else if (column == COL_SPECIALS) {
		    if (special == CHAR_SEPERATOR)
			char_seperator = file[i];
		    else if (special == CHAR_ALPHA)
			anim->char_alpha = file[i];
		    else if (special == CHAR_SPINNER)
			anim->char_spinner = file[i];

		    special++;
		}
	    }
	} else if (state == STATE_SEPERATOR) {
	    if (file[i] != char_seperator) {
		frame++;
		state++;
		x = 0; y = 0;
		i--;	// reprocess this character
		j = 0;
	    }
	} else if (state == STATE_FRAME) {
	    int is_seperator =
		(x == 0 && file[i] == char_seperator) ? 1 : 0;
	    int is_eof = ((i + 1) == st.st_size) ? 1 : 0;

	    // end of line or end of file
	    if (file[i] == '\n') {
		x = 0;
		y++;
		height = y;
		buf[j++] = file[i];
	    } else if (is_seperator || is_eof) {
		anim_frame_t *frame;
		slist_t *item;

		if (x != 0) {
		    x = 0;
		    y++;
		    height = y;
		}

		len = width * height;

		// build frame
		frame = malloc(sizeof(anim_frame_t));
		frame->width         = width;
		frame->height        = height;
		frame->spinner_state = 1;
		frame->data          = malloc(len);

		// store data in frame
		assert(frame->data != NULL);

		// fill in data frame
		x = y = 0;
		for (k = 0; k < j; k++) {
		    if (buf[k] == '\n') {
			x = 0;
			y++;
			continue;
		    }
		    *(frame->data + (y * width) + x) = buf[k];
		    x++;
		}

		// append frame to list
		item = slist_new();
		item->data = frame;

		anim->frames = slist_append(anim->frames, item);
		anim->n_frames++;

		if (is_eof)
		    break;
		else
		    state--;
	    } else {
		buf[j++] = file[i];

		if (++x > width)
		    width = x;
	    }
	}

    }

    return anim;
}

anim_frame_t *
anim_frame(anim_t *anim, unsigned int index)
{
    anim_frame_t *frame;
    slist_t *item;

    if (index >= anim->n_frames)
	return NULL;

    item = slist_index(anim->frames, index);
    frame = item->data;
    return frame;
}


#ifndef _GAME_H
#define _GAME_H

#include <time.h>
#include <stdint.h>

#include "slist.h"

#define CALL(x) if (x != NULL) x

typedef struct {
    time_t date_time;
    int reaction_time;
} reaction_t;

typedef struct {
    slist_t *reaction_list;
    int reactions;
    double average;
    double latest_reaction;
} game_data_t;

extern game_data_t *game_data;

#if 0
time_driver_t  *time_driver;
input_driver_t *input_driver;
term_driver_t  *term_driver;
random_driver_t *random_driver;
#endif

typedef int32_t int32_vec2_t[2];

typedef struct {
    unsigned int width;
    unsigned int height;
    char *data;
    unsigned int spinner_state;
} anim_frame_t;

typedef struct {
    char *name;			// name of asset
    slist_t *frames;		// one data set for each frame
    unsigned int n_frames;	// number of animation frames

    char char_alpha;
    char char_spinner;
    char char_passenger;
} anim_t;


int  global_keys(int key);

int  log_open(char *filename);
void log_write(int fd, time_t date_time, int reaction_time, char *delim, char *quote);

void game_update(void);
void game_shutdown(void);
anim_t *anim_load(char *filename);
anim_frame_t *anim_frame(anim_t *anim, unsigned int idx);

void game_msleep(unsigned int tv_msec);

anim_t *asset_load(char *filename);

#endif

#ifndef COLOR_H
#define COLOR_H

#include "types.h"

typedef struct {
    uint8_t id;
    char *name;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

extern color_t colors[];

// Return -1 on not found, otherwise index to colors[]
int color_name_to_id(char *name);

int color_bg();
int color_fg();

// Returns the color we set to, or -1 on failure
void color_set_bg(unsigned id);

// Returns the color we set to, or -1 on failure
void color_set_fg(unsigned id);

// For a given color ID, return the RGB triple
void color_to_rgb(unsigned id, unsigned *r, unsigned *g, unsigned *b);

#endif

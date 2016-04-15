#include "types.h"
#include "color.h"

color_t colors[] = {
  {0, "black",   0x00, 0x00, 0x00},
  {1, "white",   0xff, 0xff, 0xff},
  {2, "red",     0xff, 0x00, 0x00},
  {3, "green",   0x00, 0xff, 0x00},
  {4, "blue",    0x00, 0x00, 0xff},
  {5, "yellow",  0xff, 0xff, 0x00},
  {6, "magenta", 0xff, 0x00, 0xff},
  {7, "cyan",    0x00, 0xff, 0xff},
  {8, "def_fg",  103, 248, 111}
};


static unsigned color_background = 1;
static unsigned color_foreground = 0;


int color_name_to_id(char *name) {
    char buffer[8];
    unsigned x = 0;

    while (x < (sizeof(colors) / sizeof(colors[0]))) {
        if (strcasecmp(name, colors[x].name) == 0) {
            return(colors[x].id);
        }

        x++;
    }

    // User defined color?
    if (strlen(name) == 6) {
        sscanf(name, "%6x", &x);
        snprintf(buffer, 7, "%06x", x);
        if (strcmp(name, buffer) == 0) {
            return(x);
        }
    }

    return(-1);
}


// The background color.
int color_bg() {
    return(color_background);
}


void color_set_bg(unsigned id) {
    color_background = id;
}


// The foreground color.
int color_fg() {
    return(color_foreground);
}


void color_set_fg(unsigned id) {
    color_foreground = id;
}


void color_to_rgb(unsigned id, unsigned *r, unsigned *g, unsigned *b) {
    if (id < sizeof(colors) / sizeof(colors[0])) {
        *r = colors[id].r;
        *g = colors[id].g;
        *b = colors[id].b;
    } else {
        *r = ((id & 0xff0000) >> 16) & 0xff;
        *g = ((id & 0x00ff00) >>  8) & 0xff;
        *b = ((id & 0x0000ff)      ) & 0xff;
    }
}

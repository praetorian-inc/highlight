#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"
#include "bmp.h"


typedef struct screen_t screen_dummy;


// Create a new virtual screen of width and heigh in characters
struct screen_t *screen_new(unsigned char_width, unsigned char_height);


// Place character ch at screen location x (width), y (height).
void screen_char(struct screen_t *screen, unsigned char ch, unsigned x, unsigned y);


// Does a basic printf at the bottom of the display.  Note the only
// formatting allowed is \n character.
void screen_printf(struct screen_t *screen, char *string);


// Create a BMP file of the resulting image.
void screen_write_image(struct screen_t *screen, char *filename);


// Create a box on the screen around the chosen characters and using the given color.
void screen_draw_box(struct screen_t *screen, unsigned char_left, unsigned char_top, unsigned char_right, unsigned char_bottom, unsigned color);


// Place a character at a given location directly on the screen.
void screen_draw_character(struct screen_t *screen, unsigned char ch, unsigned x, unsigned y);


// Locate the blur string and then blur everything in the same column below and to the right of that.
void screen_blur(struct screen_t *screen, char *blur_string, unsigned wantInsensitive);


// Returns whether we successfully blurred any data.
unsigned screen_did_blur(struct screen_t *screen);


// Return count of anything found
unsigned screen_search(struct screen_t *screen, char *string, unsigned color, unsigned wantInsensitive, unsigned greedy_level);

// Adjust the context for the screen
void screen_fix_context(struct screen_t *screen, char *string, unsigned wantInsensitive, unsigned context);

// Adjust to remove leading or trailing blank lines on the screen
void screen_fix_blanklines(struct screen_t *screen);

// Not always picked up by various *nix
extern char *strcasestr(const char *haystack, const char *needle);

#endif

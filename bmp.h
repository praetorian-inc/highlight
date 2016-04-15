#ifndef BMP_H
#define BMP_H

#include "types.h"
#include "font.h"

typedef struct image_t bmp_dummy;


void bmp_set_bit(struct image_t *bmp, unsigned x, unsigned y, unsigned color);


void bmp_get_bit(struct image_t *bmp, unsigned x, unsigned y, unsigned *color);


// Caller will ask for a virtual screen, of width x height in characters
// This code will handle the padding, etc.
struct image_t *bmp_new(unsigned width, unsigned height);


void bmp_write_image(char *filename, struct image_t *bmp);

void bmp_draw_horiz_line(struct image_t *bmp, unsigned left, unsigned right, unsigned y, unsigned color);

void bmp_draw_vert_line(struct image_t *bmp, unsigned top, unsigned bottom, unsigned x, unsigned color);


// For screen drawing a new line of text, shift the image up by amount bits.
// Replaced bits will be background color.
void bmp_move_up(struct image_t *bmp, unsigned dest_row, unsigned src_row, unsigned total_rows);


// Thickness and relative placement are fixed.
void bmp_draw_box(struct image_t *bmp, unsigned top, unsigned left, unsigned right, unsigned bottom, unsigned color, unsigned thickness);

// Shrink the BMP imag
void bmp_shrink_height(struct image_t *bmp, unsigned new_height);


#endif

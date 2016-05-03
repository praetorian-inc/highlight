#include "screen.h"
#include "color.h"

// Will have to tinker with these to find a good setting.
#define PADDING_TOP    5
#define PADDING_LEFT   5
#define PADDING_RIGHT  5
#define PADDING_BOTTOM 5


/*
 * When we printf a line, typically it's terminated by a NL character. We
 * don't want to shift the screen up a line if that's the end of the
 * output - otherwise we end up with an empty line at the bottom of
 * the screen.  So we overload the x_pos.  It's -1 to indicate that
 * there is a hidden newline.  Otherwise, it's the x coord of where the
 * next string will go.
 */

struct screen_t {
    unsigned width;
    unsigned height;
    char *chars;        // Characters the user can see
    struct image_t *image;
    int x_pos;
    unsigned did_blur;  // Sometimes we just want blurring, so need to know if we blurred anything
};


typedef struct {        // Units in characters, not pixels
    unsigned exists;
    unsigned left;
    unsigned top;
    unsigned right;
    unsigned bottom;
} box_t;


struct screen_t *screen_new(unsigned char_width, unsigned char_height) {
    struct screen_t *answer = NULL;
    unsigned amount = char_width * char_height;
    unsigned bmp_width, bmp_height;

    answer = (struct screen_t *) malloc(sizeof(struct screen_t));
    assert(answer);

    answer->width = char_width;
    answer->height = char_height;

    answer->chars = (char *) malloc(amount + 1);
    assert(answer->chars);

    memset(answer->chars, ' ', amount);
    answer->chars[amount] = '\0';                   // Will make searching easier latter

    bmp_width = font_width() * char_width + PADDING_LEFT + PADDING_RIGHT;
    bmp_height = font_height() * char_height + PADDING_TOP + PADDING_BOTTOM;
    answer->image = bmp_new(bmp_width, bmp_height);
    //bmp_draw_box(answer->image, PADDING_TOP - 1, PADDING_LEFT - 1, bmp_width - PADDING_RIGHT, bmp_height - PADDING_BOTTOM, color_name_to_id("black"), 1);      // background, 1 pixel

    answer->x_pos = -1;
    answer->did_blur = false;

    return(answer);
}


// Place character ch at screen location x (width), y (height).
void screen_char(struct screen_t *screen, unsigned char ch, unsigned x, unsigned y) {
    assert(x < screen->width);
    assert(y < screen->height);
    screen->chars[y * screen->width + x] = ch;
    screen_draw_character(screen, ch, x, y);
}


static void screen_move_up(struct screen_t *screen) {
    char *dst = screen->chars;
    char *src = dst + screen->width;
    unsigned x;

    memmove(dst, src, (screen->width * (screen->height - 1)));

    // Move the BMP image up, it will use background color for fill
    bmp_move_up(screen->image, PADDING_TOP, PADDING_TOP + font_height(), (screen->height - 1) * font_height());

    // Now an empty line at the end.  This also draws the image content.
    for (x = 0; x < screen->width; x++) {
        screen_char(screen, ' ', x, screen->height - 1);
    }
}


void screen_printf(struct screen_t *screen, char *string) {
    unsigned row = screen->height - 1;

    // If we had a leftover newline, handle it by shifting screen and bitmap
    while (*string) {
        if (screen->x_pos == -1) {
            screen_move_up(screen);
            screen->x_pos = 0;
        }

        if (*string == '\n') {
            screen->x_pos = -1;
        } else if (*string == '') {
            char *p = NULL;

            // Do nothing with the escape.
            // If it's an ANSII color, silently consume it:   <ESC>[#m
            // Where # may be numbers separated by semicolons
            // This cheap regex will consume <ESC>[0-9\[;]*m
            p = string + 1;
            while ((*p == ';') || (*p == '[') || ((*p >= '0') && (*p <= '9'))) {
                p++;
            }

            if (*p == 'm') {
                string = p;         // Point at final 'm', which will be skipped
            }
        } else {
            screen_char(screen, *string, screen->x_pos++, row);

            // Hit end of line?  Wrap around and keep going.
            if (screen->x_pos == screen->width) {
                screen_move_up(screen);
                screen->x_pos = 0;
            }
        }

        string++;
    }
}


void screen_write_image(struct screen_t *screen, char *filename) {
    bmp_write_image(filename, screen->image);
}


void screen_draw_box(struct screen_t *screen, unsigned char_left, unsigned char_top, unsigned char_right, unsigned char_bottom, unsigned color) {
#define AWAY 3
#define THICKNESS 4
    unsigned width = font_width();
    unsigned height = font_height();
    unsigned top, left, right, bottom;

    left = char_left * width + PADDING_LEFT - AWAY;
    top = char_top * height + PADDING_TOP - AWAY;
    right = (char_right + 1) * width + PADDING_LEFT + AWAY - 2;
    bottom = (char_bottom + 1) * height + PADDING_TOP + AWAY - 2;

    bmp_draw_box(screen->image, top, left, right, bottom, color, THICKNESS);
}


void screen_draw_character(struct screen_t *screen, unsigned char ch, unsigned x, unsigned y) {
    char *glyph = font_char_start(ch);
    unsigned i, j, color;
    unsigned width = font_width();
    unsigned height = font_height();

    x *= width;
    x += PADDING_LEFT;

    y *= height;
    y += PADDING_TOP;

    // x and y are now starting bit positions in the image
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            color = (*glyph == 0) ? color_bg() : color_fg();
            bmp_set_bit(screen->image, x + j, y + i, color);
            glyph++;
        }
     }
}


void screen_blur(struct screen_t *screen, char *string, unsigned wantInsensitive) {
    unsigned blur_column;
    char *content = screen->chars;
    char *p = NULL;
    unsigned offset, r, c;

    if (wantInsensitive == true) {
        p = strcasestr(content, string);
    } else {
        p = strstr(content, string);
    }

    // It's not an error to not find anything to blur
    if (!p) {
        return;
    }

    offset = p - screen->chars;
    r = offset / screen->width;
    c = offset % screen->width;

    blur_column = c;

    // Starting in the next row, blur any non-space characters found in 
    // the blur_column to the end of the line.
    while (++r < screen->height) {
        offset = r * screen->width + blur_column;
        c = blur_column;

        p = content + offset;
        while (c < screen->width) {
            if (*p != ' ') {
                *p = '\x7f';        // DEL character, still 7 bits
                screen_draw_character(screen, '\x7f', c, r);
                screen->did_blur = true;
            }
            p++;
            c++;
        }
    }
}


// When we find a string, may want to expand characters before or after
static void screen_greedy_expand(struct screen_t *screen, char **found, unsigned *len, unsigned greedy_level) {
    unsigned c;
    char ch;
    char *p = NULL;

    if (greedy_level == 0) {
        return;
    }

    // Get the starting column
    c = (*found - screen->chars) % screen->width;

    // Expand before the start of the string, but not too far.
    p = *found;
    while (c-- > 0) {
        ch = *(--p);

        if (greedy_level == 1) {
            if (((ch >= 'a') && (ch <= 'z')) ||
                ((ch >= 'A') && (ch <= 'Z')) ||
                ((ch >= '0') && (ch <= '9')) ||
                ((ch == '.') || (ch == '-'))) {
                *found -= 1;
                *len += 1;
            } else {
                c = 0;
            }
        } else if (greedy_level == 2) {
            if (ch == ' ') {
                c = 0;
            } else {
                *found -= 1;
                *len += 1;
            }
        } else {
            fprintf(stderr, "Unexpected greedy level %u\n", greedy_level);
            exit(EXIT_FAILURE);
        }
    }

    // Get the end of the string
    c = ((*found - screen->chars) % screen->width) + *len;
    p = *found + *len;
    while (c++ < screen->width) {               // c is column just after string
        ch = *(p++);

        if (greedy_level == 1) {
            if (((ch >= 'a') && (ch <= 'z')) ||
                ((ch >= 'A') && (ch <= 'Z')) ||
                ((ch >= '0') && (ch <= '9')) ||
                ((ch == '.') || (ch == '-'))) {
                *len += 1;
            } else {
                c = screen->width;
            }
        } else if (greedy_level == 2) {
            if (ch == ' ') {
                c = screen->width;
            } else {
                *len += 1;
            }
        } else {
            fprintf(stderr, "Unexpected greedy level %u\n", greedy_level);
            exit(EXIT_FAILURE);
        }
    }
}


unsigned screen_did_blur(struct screen_t *screen) {
    return(screen->did_blur);
}


// Scan top down, look for boxes that can be drawn.
// Up to N boxes per line can exist at once, for widely separated boxes.
// A box isn't drawn until we are sure of the final boundaries.
// If a box is found, try to combine with a box on the previous line.
unsigned screen_search(struct screen_t *screen, char *string, unsigned color, unsigned wantInsensitive, unsigned greedy_level) {
#define MAXBOX 7
    box_t box[MAXBOX];
    char *content = screen->chars;
    char *found = NULL;
    unsigned extend;
    unsigned i, len, offset, r, c, test, count = 0;

    for (i = 0; i < MAXBOX; i++) {
        box[i].exists = false;
    }

    while (*content) {
        if (wantInsensitive == true) {
            found = strcasestr(content, string);
        } else {
            found = strstr(content, string);
        }
        if (!found) break;

        count++;

        // Adjust start and end for the greediness
        len = strlen(string);
        screen_greedy_expand(screen, &found, &len, greedy_level);

        // Get the row and column of the start of this entry.
        offset = found - screen->chars;
        r = offset / screen->width;
        c = offset % screen->width;

        // Close off any previous boxes.
        for (i = 0; i < MAXBOX; i++) {
            if (box[i].exists == true) {
                if ((r > 1) && box[i].bottom <= (r - 2)) {
                    screen_draw_box(screen, box[i].left, box[i].top, box[i].right, box[i].bottom, color);
                    box[i].exists = false;
                }
            }
        }

        // Does this selection extend a previous box?  If so, there will 
        // be some part of the box covered by the same column area.
        extend = false;
        for (i = 0; (i < MAXBOX) && (extend == false); i++) {
            if (box[i].exists) {
                if ((box[i].left <= c) && (c <= box[i].right)) {
                    extend = true;
                } else {
                    test = c + len - 1;
                    if ((box[i].left <= test) && (test <= box[i].right)) {
                        extend = true;
                    }
                }

                if (extend == true) {
                    if (c < box[i].left) {
                        box[i].left = c;
                    }

                    if ((c + len - 1) > box[i].right) {
                        box[i].right = c + len - 1;
                    }

                    box[i].bottom = r;
                }
            }
        }

        // It's a new box if extend == false
        // Find a free item
        for (i = 0; (i < MAXBOX) && (extend == false); i++) {
            if (box[i].exists == false) {
                extend = true;
                box[i].exists = true;
                box[i].left = c;
                box[i].top = r;
                box[i].right = c + len - 1;
                box[i].bottom = r;
            }
        }

        if (extend == false) {
            // Shouldn't assert.  At seven or so boxes, it's
            // not being all that useful to the customer.
            fprintf(stderr, "Ran out of boxes at row: %u, col: %u, string: %s\n", r, c, string);
        }

        // So we can search for the next string.
        content = found + len;
    }

    // Any remaining boxes get drawn.
    for (i = 0; i < MAXBOX; i++) {
        if (box[i].exists == true) {
            screen_draw_box(screen, box[i].left, box[i].top, box[i].right, box[i].bottom, color);
            box[i].exists = false;
        }
    }

    return(count);
}


// Adjust the screen content based on the desired context.
// Phase 1: Find first match, shift screen up as needed.
// Phase 2: Find last match, truncate the screen and bmp file to 
//          right size (will also redo the implicit border)
void screen_fix_context(struct screen_t *screen, char *string, unsigned wantInsensitive, unsigned context) {
    char *content = screen->chars;
    char *prev = NULL;
    char *found = NULL;
    unsigned offset, r;

    // Phase 1
    if (g_verbose > 2) {
        fprintf(stderr, "%s:%u starting phase 1\n", __FILE__, __LINE__);
    }

    if (wantInsensitive == true) {
        found = strcasestr(content, string);
    } else {
        found = strstr(content, string);
    }

    // It's not an error if we didn't find the string.
    if (found == NULL) {
       return;
    }

    // Regardless of the greedy level, we won't change rows.

    // Get the row of the entry.
    offset = found - screen->chars;
    r = offset / screen->width;

    if (g_verbose > 2) {
        fprintf(stderr, "%s:%u offset: %u, r: %u\n", __FILE__, __LINE__, offset, r);
    }

    // If we have too many rows at the top, shift the screen up
    while (r-- > context) {
        screen_printf(screen, "\n");
    }

    // Phase 2, there's no backward str[case]str
    if (g_verbose > 2) {
        fprintf(stderr, "%s:%u starting phase 2\n", __FILE__, __LINE__);
    }
    prev = NULL;
    content = screen->chars;        // Have to start over
    while (*content) {
        if (wantInsensitive == true) {
            found = strcasestr(content, string);
        } else {
            found = strstr(content, string);
        }
        
        // Scan until we can't find any more.
        if (found == NULL) {
            break;
        } else {
            prev = found;
        }

        // Don't need to worry about greedy level here.
        content = found + 1;
    }

    // We should have found our first value no matter what.
    assert(prev);

    // Get the row of the entry.
    offset = prev - screen->chars;
    r = offset / screen->width;

    if (g_verbose > 2) {
        fprintf(stderr, "%s:%u offset: %u, r: %u\n", __FILE__, __LINE__, offset, r);
    }

    // This will be the last row
    r += context;
    if (r > screen->height) {
        return;
    }

    // Adjust the screen height.  We don't need to reallocate memory.
    screen->height = r + 1;

    // The adjusting of the bitmap will be later
}


// Return true if the given row is blank
static int row_is_blank(struct screen_t *screen, unsigned row) {
    unsigned i;
    char *p = screen->chars;

    p += row * screen->width;
    for (i = 0; i < screen->width; i++) {
        if (*(p++) != ' ') {
            return(false);
        }
    }

    return(true);
}


void screen_fix_blanklines(struct screen_t *screen) {
    unsigned bmp_height, bmp_width;
    unsigned row = screen->height;

    // For any blank rows, shift the screen up, just not too far
    while (row > 0) {
        if (row_is_blank(screen, 0) == true) {
            screen_printf(screen, "\n");
            row--;
        } else {
            break;
        }
    }

    // Starting at the bottom, work backwards to the first non-blank
    // row, or the first line of the display.
    row = screen->height;

    while (row > 1) {
        row--;
        if (row_is_blank(screen, row) == false) {
            break;
        }
    }

    screen->height = row + 1;

    // Tell BMP to readjust the height and redraw the border.
    bmp_height = font_height() * screen->height + PADDING_TOP + PADDING_BOTTOM;
    if (g_verbose > 2) {
        fprintf(stderr, "%s:%u new bmp_height %u\n", __FILE__, __LINE__, bmp_height);
    }
    bmp_shrink_height(screen->image, bmp_height);

    bmp_width = font_width() * screen->width + PADDING_LEFT + PADDING_RIGHT;

    // Redraw the border
    bmp_draw_box(screen->image, 0, 0, bmp_width - 1, bmp_height - 1, color_name_to_id("black"), 2);      // black border
}

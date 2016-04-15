#ifndef FONT_H
#define FONT_H

// So we can find out the width and height of characters.
// It's expected the padding is already built in.

unsigned font_width();
unsigned font_height();
char *font_char_start(unsigned char ch);
#endif

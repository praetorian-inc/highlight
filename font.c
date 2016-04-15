#include "types.h"
#include "font.h"
#include "monaco_compressed_large.h"

char *font_data = NULL;


unsigned 
font_width() {
    return(14); 
}


unsigned 
font_height() { 
    return(23); 
}


static void font_fill() {
    char *dst = NULL;
    char *src = font_compressed;
    unsigned amount = (128 - 32) * 23 * 14;
    unsigned byte, i, zero = 0;

    font_data = (char *) malloc(amount);
    assert(font_data);
    dst = font_data;

    while (amount) {
        if (zero) {
            byte = 0;
            zero--;
        } else {
            if (*src == 'Z') {
                zero = (*(src + 1) - '0') - 1;
                byte = 0;
                src += 2;
            } else {
                sscanf(src, "%02x", &byte);
                src += 2;
            }
        }

        for (i = 0; i < 8; i++) {
            *(dst++) = (byte & 0x80) >> 7;
            byte <<= 1;
        }
        amount -= 8;
    }
}


char *font_char_start(unsigned char ch) {
    unsigned offset;

    if (font_data == NULL) {
        font_fill();
    }

    if ((ch < ' ') || (ch > '\x7f')) {
        ch = ' ';
    }

    offset = (font_width() * font_height()) * (ch - ' ');
    return(font_data + offset);
}

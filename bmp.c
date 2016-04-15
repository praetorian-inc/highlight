// From: http://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries

#include "types.h"
#include "color.h"
#include "bmp.h"
#include "font.h"

struct image_t {
    unsigned bit_width;
    unsigned bit_height;
    uint8_t *data;
};


void bmp_set_bit(struct image_t *bmp, unsigned x, unsigned y, unsigned color) {
    uint8_t *dp = NULL;

    dp = bmp->data + x + (y * bmp->bit_width);
    *dp = color;
}


void bmp_get_bit(struct image_t *bmp, unsigned x, unsigned y, unsigned *color) {
    uint8_t *dp = NULL;

    dp = bmp->data + x + (y * bmp->bit_width);
    *color = *dp;
}


// Caller will ask for a virtual screen, of width x height in characters
// This code will handle the padding, etc.
struct image_t *bmp_new(unsigned width, unsigned height) {
    unsigned amount, color;
    struct image_t *answer = NULL;
    uint8_t *p;

    answer = (struct image_t *) malloc(sizeof(struct image_t));
    assert(answer);

    answer->bit_width = width;
    answer->bit_height = height;

    amount = answer->bit_width * answer->bit_height;

    answer->data = (uint8_t *) malloc(amount);
    assert(answer->data);

    p = answer->data;

    // Set background
    color = color_bg();
    while (amount--) {
        *(p++) = color;
    }

    return(answer);
}


// Write a line of the BMP file out as a chunk.
static void writebits(FILE *fp, uint8_t *p, unsigned remain) {
    int amount;

    while (remain > 0) {
        amount = fwrite(p, 1, remain, fp);
        assert(amount > 0);
        
        p += amount;
        remain -= amount;
    }
}


// Generate a run length compresion of the BMP file.  Don't need
// the optimal compression here, jsut something pretty good. 
// Could use the existing image for this, but there's no need.
static uint8_t *bmp_make_compressed(struct image_t *bmp, unsigned *compressed_len) {
    unsigned rows, amount;
    uint8_t *start = NULL;
    uint8_t *s = NULL;
    uint8_t *d = NULL;
    uint8_t *max = NULL;
    unsigned color = 0;
    unsigned ct = 0;

    // Compression will be done so that it will take less room. 
    // But no need to be cute here by taking over the same
    // area as the current image.
    amount = bmp->bit_width * bmp->bit_height;
    start = (uint8_t *) malloc(amount);
    assert(start);

    // Will test as we go
    max = start + amount;

    // In general:
    // 1st pass per line: (expect lots of repeated backgroun)
    // <ct> <color>
    //
    // 2nd pass, more compression, needs 6 bytes to compress
    // 00 N (>=3) <colors> [optional 00 for 16 bit padding]
    // -- NOTE: Not clear this will be needed, so skip for now
    
    // 00 00 end of line
    // 00 01 end of bitmap

    d = start;
    for (rows = 0; rows < bmp->bit_height; rows++) {
        // BMP is upsidedown
        s = bmp->data + (bmp->bit_height - rows - 1) * bmp->bit_width;

        ct = 0;     // And color == undefined
        amount = bmp->bit_width;
        while (amount-- > 0) {
            if (ct == 255) {
                // Maximum for this run, so store it.
                *(d++) = ct;
                *(d++) = color;
                assert(d < max);

                ct = 0;     // And color == undefined
            }

            if (ct == 0) {
                // Get the color and start counting
                color = *(s++);
                ct = 1;
            } else {
                // If same color, increment count, won't ever be > 255
                if (color == *s) {
                    ct++;
                } else {
                    *(d++) = ct;
                    *(d++) = color;
                    assert(d < max);

                    // New entry
                    ct = 1;
                    color = *s;
                }

                s++;
            }
        }

        // Leftover
        if (ct) {
            *(d++) = ct;
            *(d++) = color;
            assert(d < max);
        }

        // End of line
        *(d++) = 0;
        *(d++) = 0;
        assert(d < max);
    }

    // End of bitmap
    *(d++) = 0;
    *(d++) = 1;
    assert(d < max);

    // Figure out the length used, note that we may use less.
    *compressed_len = d - start;
    return(start);
}


void bmp_write_image(char *filename, struct image_t *bmp) {
    unsigned int headers[13];
    FILE *outfile = NULL;
    unsigned width = bmp->bit_width;
    unsigned height = bmp->bit_height;
    unsigned int paddedsize;
    int n;
    unsigned r, g, b;
    uint8_t *compressed = NULL;
    unsigned compressed_len = 0;

    compressed = bmp_make_compressed(bmp, &compressed_len);
    assert(compressed);

    // Header + color table (RGB and Alpha)
    paddedsize = 54 + (256 * 4);

    headers[0]  = paddedsize + compressed_len;  // bfSize (whole file size)
    headers[1]  = 0;                            // bfReserved (both)
    headers[2]  = paddedsize;                   // bfOffbits
    headers[3]  = 40;                           // biSize
    headers[4]  = width;                        // biWidth
    headers[5]  = height;                       // biHeight
    headers[6]  = 0x00080001;                   // biPlanes and biBitCounts are fixed
    headers[7]  = 1;                            // biCompression -  8 bit RLE
    headers[8]  = compressed_len;               // biSizeImage
    headers[9]  = 0;                            // biXPelsPerMeter
    headers[10] = 0;                            // biYPelsPerMeter
    headers[11] = 256;                          // biClrUsed
    headers[12] = 256;                          // biClrImportant

    outfile = fopen(filename, "wb");

    //
    // Headers begin...
    // Write out 1 character at a time to avoid endian issues.
    //

    fprintf(outfile, "BM");

    for (n = 0; n < (sizeof(headers) / sizeof(headers[0])); n++) {
        fprintf(outfile, "%c", headers[n] & 0x000000FF);
        fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
        fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
        fprintf(outfile, "%c", (headers[n] & 0xFF000000) >> 24);
    }

    // Now do the color table
    for (n = 0; n < 256; n++) {
        color_to_rgb(n, &r, &g, &b);
        fprintf(outfile, "%c", b);
        fprintf(outfile, "%c", g);
        fprintf(outfile, "%c", r);
        fprintf(outfile, "%c", 0);
    }

    // And blather out the compressed table
    writebits(outfile, compressed, compressed_len);

    fclose(outfile);
    free(compressed);  compressed = NULL;
    return;
}


void bmp_draw_horiz_line(struct image_t *bmp, unsigned left, unsigned right, unsigned y, unsigned color) {
    assert(left <= right);

    while (left <= right) {
        bmp_set_bit(bmp, left++, y, color);
    }
}


void bmp_draw_vert_line(struct image_t *bmp, unsigned top, unsigned bottom, unsigned x, unsigned color) {
    assert(top <= bottom);

    while (top <= bottom) {
        bmp_set_bit(bmp, x, top++, color);
    }
}



void bmp_draw_box(struct image_t *bmp, unsigned top, unsigned left, unsigned right, unsigned bottom, unsigned color, unsigned thickness) {
    while (thickness-- != 0) {
        bmp_draw_horiz_line(bmp, left, right, top, color);
        bmp_draw_horiz_line(bmp, left, right, bottom, color);
        bmp_draw_vert_line(bmp, top, bottom, left, color);
        bmp_draw_vert_line(bmp, top, bottom, right, color);

        left++;
        right--;
        top++;
        bottom--;
    }
}


void bmp_move_up(struct image_t *bmp, unsigned dest_row, unsigned src_row, unsigned total_rows) {
    uint8_t *dp = NULL;
    uint8_t *sp = NULL;

    dp = bmp->data + dest_row * bmp->bit_width;
    sp = bmp->data + src_row * bmp->bit_width;
    
    // Copy line sp into dp until sp goes off the end.
    while (total_rows-- < bmp->bit_height) {
        memcpy(dp, sp, bmp->bit_width);
        dp += bmp->bit_width;
        sp += bmp->bit_width;
    }
}


// Shrink the BMP image by removing the rows at the bottom
void bmp_shrink_height(struct image_t *bmp, unsigned new_height) {
    // Shouldn't happen.
    assert(bmp->bit_height >= new_height);

    // Just change the height, don't need to reallocate the screen.
    bmp->bit_height = new_height;
}


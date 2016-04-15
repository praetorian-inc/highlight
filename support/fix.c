/*
 * Stand alone program to generate a compressed version of the font table.
 * Basically hex with optimization for zero byte runs.
 */

#include <stdio.h>
#include "monaco_large.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *src = font_data;
    unsigned amount = (128 - 32) * 23 * 14;
    unsigned byte, i, zero = 0;

    printf("static char *font_compressed = \"");

    while (amount) {
        byte = 0;
        for (i = 0; i < 8; i++) {
            byte <<= 1;
            byte |= *(src++);
        }

        if (byte == 0) {
           zero++;
        } else {
            if (zero) {
                printf("Z%c", (zero + '0'));
                if ((zero + '0') == '\\') {
                    printf("\\");
                }
                zero = 0;
            }
            printf("%02x", byte);
        }

        amount -= 8;
    }

    if (zero) {
        printf("Z%c", (zero + '0'));
    }
    printf("\";\n");
}

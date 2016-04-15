#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "types.h"
#include "color.h"
#include "screen.h"

// Declared in types.h
int g_verbose = 0;

// The options, listed in the same order as the code
typedef struct {
    unsigned bg;
    unsigned context;
    unsigned width;
    unsigned height;
    unsigned fg;
    unsigned greedy;
    unsigned wantInsensitive;
    char *ofile;
    char *blur_string;
    unsigned box_color;
    char *search_string;
} options_t;


void usage(char *program) {
    fprintf(stderr, "Usage: %s [options] <string to find>\n", program);
    fprintf(stderr, " -b color   Background color (default black)\n");
    fprintf(stderr, " -c int     Keep this many lines before and after found for context\n");
    fprintf(stderr, " -d WxH     Dimensions as <width>x<height, default 80x25\n");
    fprintf(stderr, " -f color   Foreground color (default light green)\n");
    fprintf(stderr, " -h         Help\n");
    fprintf(stderr, " -g int     Greedy consuption of strings that are found:\n");
    fprintf(stderr, "              0 (default) exact strings\n");
    fprintf(stderr, "              1 extend leading and trailing alphabet and numbers\n");
    fprintf(stderr, "              2 extend leading and trailing until spacing\n");
    fprintf(stderr, " -i         Case insensitive search\n");
    fprintf(stderr, " -o file    Output image to a file (when matches found or blurring)\n");
    fprintf(stderr, " -r string  Blur everything below this found string (i.e. Password)\n");
    fprintf(stderr, " -v int     Verbose level (default 0), larger is more\n");
    fprintf(stderr, " -x color   Box color (default red)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "At least blur (-r) or a search string must be specified.\n");
    fprintf(stderr, "Colors may be specified as an RGB tuple, i.e. -f c0ffee\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Returns number of matches in the $? shell variable.\n");

    exit(EXIT_FAILURE);
}


void parse_opts(options_t *options, int argc, char *argv[]) {
    int opt;
    int val;
    char *p = NULL;

    // Set defaults that the user may override
    options->bg = color_name_to_id("black");
    options->context = 0;
    options->width = 80;
    options->height = 25;
    options->fg = color_name_to_id("def_fg");
    options->greedy = 0;
    options->wantInsensitive = false;
    options->ofile = NULL;
    options->blur_string = NULL;
    options->box_color = color_name_to_id("red");
    options->search_string = NULL;
    
    // Scan the user supplied options
    while ((opt = getopt(argc, argv, "b:c:d:f:g:hio:r:v:x:")) != -1) {
        switch(opt) {
        case 'b':
            val = color_name_to_id(optarg);

            if (val == -1) {
                fprintf(stderr, "Unknown color for background\n");
                usage(argv[0]);
            }

            options->bg = val;
            break;

        case 'c':
            options->context = atoi(optarg);
            break;

        case 'd':
            p = strchr(optarg, 'x');
            if (p == NULL) {
                usage(argv[0]);
            }
            *p = '\0';
            options->width = atoi(optarg);
            options->height = atoi(p + 1);
            
            if ((options->width == 0) || (options->height == 0) || (options->width > 1023) || (options->height > 1023)) {
                usage(argv[0]);
            }
            
            break;

        case 'f':
            val = color_name_to_id(optarg);

            if (val == -1) {
                fprintf(stderr, "Unknown color for foreground\n");
                usage(argv[0]);
            }

            options->fg = val;
            break;

        case 'g':
            options->greedy = atoi(optarg);
            break;

        case 'h':
            usage(argv[0]);
            break;

        case 'i':
            options->wantInsensitive = true;
            break;

        case 'o':
            options->ofile = optarg;
            break;

        case 'r':
            options->blur_string = optarg;
            break;

        case 'v':
            g_verbose = atoi(optarg);
            break;

        case 'x':
            val = color_name_to_id(optarg);

            if (val == -1) {
                fprintf(stderr, "Unknown color for box\n");
                usage(argv[0]);
            }

            options->box_color = val;
            break;

        default:
            usage(argv[0]);
            break;
        }
    }

    // We expect just the search string at this point.
    if (optind == (argc - 1)) {
        options->search_string = argv[optind];
    } else if (optind < argc) {
        fprintf(stderr, "Unexpected strings at the end.  Are you missing quotes?\n");
        usage(argv[0]);
    } else {
        // No search string was given, expect at least blur.
        if (options->blur_string == NULL) {
            fprintf(stderr, "Missing search string\n");
            usage(argv[0]);
        }
    }
}


int main(int argc, char *argv[]) {
#define BUFSIZE 1023
    options_t options;
    struct screen_t *screen = NULL;
    char buffer[BUFSIZE + 1];
    int result = 0;

    parse_opts(&options, argc, argv);
    if (g_verbose) {
        fprintf(stderr, "Verbosity level: %u\n", g_verbose);
    }

    (void) color_set_bg(options.bg);
    (void) color_set_fg(options.fg);

    screen = screen_new(options.width, options.height);

    // Get input until caller says to stop.
    while (fgets(buffer, BUFSIZE, stdin)) {
        screen_printf(screen, buffer);
    }

    if (options.context) {
        screen_fix_context(screen, options.search_string, options.wantInsensitive, options.context);
    }

    screen_fix_blanklines(screen);

    // Blur any data that's asked.
    if (options.blur_string) {
        screen_blur(screen, options.blur_string, options.wantInsensitive);
    }

    // Search and highlight text we want to see.
    if (options.search_string) {
        result = screen_search(screen, options.search_string, options.box_color, options.wantInsensitive, options.greedy);
    }

    // Return count of strings we found if we found any.
    if (result > 0) {
        // Write the image if we want it and there are matches found
        if (options.ofile) {
            screen_write_image(screen, options.ofile);
        }
        exit(result);
    } else {
        // We didn't find any strings.  If we blurred data, that's a success.
        if (screen_did_blur(screen)) {
            if (options.ofile) {
                screen_write_image(screen, options.ofile);
            }
            exit(0);
        } else {
            exit(-1);
        }
    }
}

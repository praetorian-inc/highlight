#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char uint8_t;

#ifndef false
#define false (0)
#define true (!false)
#endif

#define TRACE printf("%s:%u\n", __FILE__, __LINE__);
extern int g_verbose;

#endif

#ifndef _UTF8_H
#define _UTF8_H

#include <stdint.h>

typedef struct {
    uint32_t c;
    uint8_t str[5];
    uint8_t size;
} utf8_char_t;

void unicode_to_utf8(uint32_t u, utf8_char_t *utf8_char);

#endif

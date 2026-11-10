#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

typedef struct {
    int w;
    int h;
    // The glyph bitmaps are const tables (font_ascii_8x16 is const unsigned
    // char, the embedded unicode_font16x16 blob is const uint8_t), so the
    // pointer must be const too. Assigning a const array into a plain
    // uint8_t * is -Wdiscarded-qualifiers, which IDF 6 turns into an error.
    const uint8_t *data;
} glyph_t;

extern const unsigned char font_ascii_8x16[];
extern const uint8_t unicode_font16x16_start[] asm("_binary_unicode_font16x16_bin_start");
extern const uint8_t unicode_font16x16_end[] asm("_binary_unicode_font16x16_bin_end");

#endif  // __FONT_H__

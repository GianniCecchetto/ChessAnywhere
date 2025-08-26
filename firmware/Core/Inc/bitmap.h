#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

#define BITMAP_SIZE				64 // sizeof(uint64_t) * 8

/*
 * Bitmap set a bit value
 */
void bitmap_set_bit(uint64_t *bitmap, int index);

/*
 * Bitmap clear a bit value
 */
void bitmap_clear_bit(uint64_t *bitmap, int index);

/*
 * Bitmap get a bit value
 */
int bitmap_get_bit(uint64_t bitmap, int index);

#endif

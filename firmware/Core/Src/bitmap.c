#include "bitmap.h"

// Typical bitmap declaration for those function : uint8_t bitmap[BITMAP_SIZE] = {0};

/*
 * Bitmap Check if the index is out of bound
 */
uint8_t is_index_out_of_bound(int index) {
	// 0 to 63
	return (index < 0 || index >= BITMAP_SIZE);
}

/*
 * Bitmap set a bit value
 */
void bitmap_set_bit(uint64_t *bitmap, int index) {
	if(is_index_out_of_bound(index)) return;
    *bitmap |= (1ULL << index);
}

/*
 * Bitmap clear a bit value
 */
void bitmap_clear_bit(uint64_t *bitmap, int index) {
	if(is_index_out_of_bound(index)) return;
	*bitmap &= ~(1ULL << index);
}

/*
 * Bitmap get a bit value
 */
int bitmap_get_bit(uint64_t bitmap, int index) {
	if(is_index_out_of_bound(index)) return -1;
	return (bitmap >> index) & 1;
}

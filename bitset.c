#include "bitset.h"


Bitset_Uint
bitset_get_uint(Bitset bs, unsigned idx, unsigned width) {
	return (bs & bit_mask(idx, width)) >> (idx);
}

/* NOTE: not compatible with BIG endian machine */
Bitset_Int
bitset_get_int(Bitset bs, unsigned idx, unsigned width) {
	Bitset_Uint raw = bitset_get_uint(bs, idx, width);

	/* if MSB is 1, we have a negative, we just need to shift it */
	if (bit(width - 1) & raw) {
		//bitset_clr(&raw, width - 1);
		Bitset negative_mask = bit_mask(width - 1, MAX_BITS - (width - 1));
		raw |= negative_mask;
	}

	return (Bitset_Int)raw;
}

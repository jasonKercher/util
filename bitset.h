#ifndef BITSET_H
#define BITSET_H

#include <stdint.h>
#include "util.h"

#if POINTER_SIZE <= 2

typedef uint8_t  Bitset8;
typedef uint16_t Bitset16;

typedef uint16_t Bitset;
typedef uint16_t Bitset_Uint;
typedef int16_t  Bitset_Int;

#elif POINTER_SIZE == 4

typedef uint8_t  Bitset8;
typedef uint16_t Bitset16;
typedef uint32_t Bitset32;

typedef uint32_t Bitset;
typedef uint32_t Bitset_Uint;
typedef int32_t  Bitset_Int;

#elif POINTER_SIZE == 8

typedef uint8_t  Bitset8;
typedef uint16_t Bitset16;
typedef uint32_t Bitset32;
typedef uint64_t Bitset64;

typedef uint64_t Bitset;
typedef uint64_t Bitset_Uint;
typedef int64_t  Bitset_Int;

#endif

#define MAX_BITS (sizeof(Bitset) * 8)
#define ALL_ONES ((Bitset)-1)

/* BS_: bitset
 * BI_: bit index
 * W_:  width
 * V_:  value
 */

#define bitset_get_ones(W_)  ((1U << (W_)) - 1)
#define bit_mask(BI_, W_)    (bitset_get_ones(W_) << (BI_))
#define bit(BI_)             (1U << (BI_))
#define bitset_get(BS_, BI_) (!!((BS_)&bit(BI_)))
#define bitset_set(BS_, BI_) (*(BS_)) |= bit(BI_)
#define bitset_clr(BS_, BI_) (*(BS_)) &= ~(bit(BI_))
#define bitset_inv(BS_, BI_) (*(BS_)) ^= bit(BI_)

#define bitset_eq(BS0_, BS1_, BI_) (bitset_get(BS0_, BI_) == bitset_get(BS1_, BI_))

#define bitset_write(BS_, BI_, V_)            \
	{                                     \
		if (V_) {                     \
			bitset_set(BS_, BI_); \
		} else {                      \
			bitset_clr(BS_, BI_); \
		}                             \
	}

Bitset_Uint bitset_get_uint(Bitset, unsigned index, unsigned width);
Bitset_Int  bitset_get_int(Bitset, unsigned index, unsigned width);

#endif /* BITSET_H */

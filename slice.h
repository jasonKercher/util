#ifndef STRINGVIEW_H
#define STRINGVIEW_H

#include <stdint.h>
#include "util.h"

struct Slice {
	void* data;
	size_t len;
};
typedef struct Slice Slice;

#define Slice(T_)             \
	struct {              \
		T_*     data; \
		ssize_t len;  \
	}

typedef Slice(uint8_t) Byte_Slice;
typedef Slice(const uint8_t) Const_Byte_Slice;
typedef Slice(char) Char_Slice;
typedef Slice(const char) Const_Char_Slice;


Slice slice_new_(unsigned size, size_t len);
#define slice_new(T_, LEN_) \
	{ .data = heap_alloc(sizeof(T_) * LEN_), .len = LEN_ }
#define slice_make(P_, LEN_) \
	{ .data = P_, .len = LEN_ }
#define slice_slice(P_, BEGIN_, END_) \
	{ .data = &(P_)[BEGIN_], .len = (END_) - (BEGIN_) }
int slice_compare(const Slice* restrict, const Slice* restrict);
int slice_compare_nocase(const Slice* restrict, const Slice* restrict);
int slice_compare_rtrim(const Slice*, const Slice*);
int slice_compare_nocase_rtrim(const Slice*, const Slice*);

#endif /* STRINGVIEW_H */

#include "bitvec.h"

#include <string.h>
#include <stdio.h>

Bitvec*
bitvec_construct(Bitvec* bv) {
	vec_construct(bv);
	memset(bv->data, 0, vec_elem_size(*bv) * bv->_cap);
	return bv;
}

void
bitvec_destroy(Bitvec* bv) {
	vec_destroy(bv);
}

bool
bitvec_at(Bitvec bv, int idx) {
	uint32_t num    = vec_at(bv, idx / 32);
	uint8_t  offset = idx % 32;
	return (num >> offset) & 1;
}

void
bitvec_set(Bitvec* bv, int idx) {
	uint32_t* num = vec_iter_at(*bv, idx / 32);
	*num |= (1 << idx % 32);
}

void
bitvec_unset(Bitvec* bv, int idx) {
	uint32_t* num = vec_iter_at(*bv, idx / 32);
	*num &= ~(1 << idx % 32);
}

void
bitvec_reserve(Bitvec* bv, int len) {
	int alloc = len / 32 + 1;
	if (bv->_cap >= ++alloc) {
		return;
	}
	void* new_dest = realloc(bv->data, alloc * vec_elem_size(*bv));
	if (!new_dest) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}
	bv->data = new_dest;
	bv->_cap = alloc;
}

void
bitvec_resize(Bitvec* bv, int len) {
	int org_size  = bv->len / 32 + 1;
	int org_alloc = bv->_cap;

	if (len > bv->len) {
		bitvec_reserve(bv, len);
	}
	bv->len = len;

	if (org_alloc == bv->_cap) {
		return;
	}
	void* new_shit      = vec_iter_at(*bv, org_size);
	int   new_shit_size = bv->_cap - org_size;
	memset(new_shit, 0, new_shit_size * vec_elem_size(*bv));
}

void
bitvec_push_back(Bitvec* bv, bool val) {
	if (++bv->len > (bv->_cap - 1) * 32) {
		bitvec_reserve(bv, bv->len * 2);
	}

	if (val) {
		bitvec_set(bv, bv->len - 1);
	} else {
		bitvec_unset(bv, bv->len - 1);
	}
}

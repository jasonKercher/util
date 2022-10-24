#include "bitvec.h"

#include <string.h>
#include <stdio.h>

Bitvec* bitvec_construct(Bitvec* bv)
{
	vec_construct(bv);
	memset(bv->data, 0, vec_elem_size(*bv) * bv->_alloc);
	return bv;
}

void bitvec_destroy(Bitvec* bv)
{
	vec_destroy(bv);
}

bool bitvec_at(Bitvec bv, int idx)
{
	uint32_t num = vec_at(bv, idx / 32);
	uint8_t offset = idx % 32;
	return (num >> offset) & 1;
}

void bitvec_set(Bitvec* bv, int idx)
{
	uint32_t* num = vec_iter_at(*bv, idx / 32);
	*num |= (1 << idx % 32);
}

void bitvec_unset(Bitvec* bv, int idx)
{
	uint32_t* num = vec_iter_at(*bv, idx / 32);
	*num &= ~(1 << idx % 32);
}

void bitvec_reserve(Bitvec* bv, int size)
{
	int alloc = size / 32 + 1;
	if (bv->_alloc >= ++alloc) {
		return;
	}
	void* new_dest = realloc(bv->data, alloc * vec_elem_size(*bv));
	if (!new_dest) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}
	bv->data = new_dest;
	bv->_alloc = alloc;
}

void bitvec_resize(Bitvec* bv, int size)
{
	int org_size = bv->size / 32 + 1;
	int org_alloc = bv->_alloc;

	if (size > bv->size) {
		bitvec_reserve(bv, size);
	}
	bv->size = size;

	if (org_alloc == bv->_alloc) {
		return;
	}
	void* new_shit = vec_iter_at(*bv, org_size);
	int new_shit_size = bv->_alloc - org_size;
	memset(new_shit, 0, new_shit_size * vec_elem_size(*bv));
}

void bitvec_push_back(Bitvec* bv, bool val)
{
	if (++bv->size > (bv->_alloc - 1) * 32) {
		bitvec_reserve(bv, bv->size * 2);
	}

	if (val) {
		bitvec_set(bv, bv->size - 1);
	} else {
		bitvec_unset(bv, bv->size - 1);
	}
}

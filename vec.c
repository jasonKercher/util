#include "vec.h"

/* The functions in here should usually never be called directly */

#include <string.h>
#include <stdio.h>
#include "util.h"

#define vec_get_idx_(v_, pos_, elem_size_) \
	_iter_size_(vec_begin(*v_), (uint8_t*)pos_, elem_size_) - 1

#define _iter_size_(begin_, back_, elem_size_) \
	(((uint8_t*)back_ - (uint8_t*)begin_) / elem_size_) + 1

/** Create **/
void* vec_construct_(void* gen_v, int elem_size) {
	Vec* v = gen_v;
	*v     = (Vec) {
            ._alloc = VEC_ALLOC_DEFAULT,
            .data   = heap_alloc(VEC_ALLOC_DEFAULT * elem_size),
        };
	return v;
}

void vec_clone_(void* gen_dest, void* gen_src, int elem_size) {
	vec_construct_(gen_dest, elem_size);
	vec_extend_(gen_dest, gen_src, elem_size);
}

void vec_copy_(void* gen_dest, void* gen_src, int elem_size) {
	Vec* v = gen_dest;
	v->size = 0;
	vec_extend_(gen_dest, gen_src, elem_size);
}

/** Iterators **/
void* vec_iter_at_(const void* gen_v, int index, int elem_size) {
	const Vec* v = gen_v;
	return v->data + elem_size * index;
}

void* vec_back_(const void* gen_v, int elem_size) {
	const Vec* v = gen_v;
	if (v->size == 0) {
		return v->data;
	}
	return vec_iter_at_(v, v->size - 1, elem_size);
}

void* vec_end_(const void* gen_v, int elem_size) {
	const Vec* v = gen_v;
	return vec_iter_at_(v, v->size, elem_size);
}

/** Resizing **/
void vec_reserve_(void* gen_v, int alloc, int elem_size) {
	Vec* v = gen_v;
	if (v->_alloc >= alloc + 1) {
		return;
	}
	void* data = realloc(v->data, (alloc + 1) * elem_size);
	if (!data) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}
	v->data   = data;
	v->_alloc = alloc + 1;
}

void vec_resize_(void* gen_v, int n, int elem_size) {
	Vec* v = gen_v;
	vec_reserve_(v, n, elem_size);
	v->size = n;
}

void vec_resize_and_zero_(void* gen_v, int size, int elem_size) {
	Vec* v         = gen_v;
	int  org_size  = v->size;
	int  org_alloc = v->_alloc;
	vec_reserve_(v, size, elem_size);
	v->size = size;
	if (org_alloc != v->_alloc) {
		int zero_size = v->_alloc - org_size;
		memset(&v->data[org_size], 0, zero_size * elem_size);
	}
}

/** Growing **/
void* vec_add_one_(void* gen_v, int elem_size) {
	Vec* v = gen_v;
	if (v->_alloc <= ++v->size) {
		vec_reserve_(v, v->_alloc * 2, elem_size);
	}
	return v->data + elem_size * (v->size - 1);
}

void* vec_add_one_front_(void* gen_v, int elem_size) {
	Vec* v          = gen_v;
	int  move_size_ = elem_size * (v->_alloc - 1);
	vec_add_one_(v, elem_size);
	memmove(v->data + elem_size, v->data, move_size_);
	return v->data;
}

void vec_push_back_(void* gen_v, const void* item, int elem_size) {
	Vec* v = gen_v;
	memcpy(vec_add_one_(v, elem_size), item, elem_size);
}

/** Assignment **/
void vec_set_at_(void* gen_v, int idx, const void* src, int n, int elem_size) {
	Vec*  v    = gen_v;
	void* dest = vec_iter_at_(v, idx, elem_size);
	memcpy(dest, src, elem_size * n);
}

/** Insertion **/
void vec_insert_iter_(
    void* gen_v, void* pos, const void* begin, const void* back, int elem_size) {
	Vec* v          = gen_v;
	int  idx        = vec_get_idx_(v, pos, elem_size);
	int  iter_size  = _iter_size_(begin, back, elem_size);
	int  iter_bytes = elem_size * iter_size;

	vec_resize_(v, v->size + iter_size, elem_size);
	int move_bytes = elem_size * (v->_alloc - idx - iter_size);

	pos = vec_iter_at_(v, idx, elem_size);

	memmove((uint8_t*)pos + iter_bytes, pos, move_bytes);
	memcpy(pos, begin, iter_bytes);
}

void vec_insert_one_(void* gen_v, void* pos, const void* item, int elem_size) {
	Vec* v = gen_v;
	vec_add_one(v);
	int move_bytes = elem_size * ((v->_alloc - 1) - vec_get_idx_(v, pos, elem_size));

	memmove((uint8_t*)pos + elem_size, pos, move_bytes);
	memcpy(pos, item, elem_size);
}

void vec_insert_one_at_(void* gen_v, int idx, const void* item, int elem_size) {
	Vec* v = gen_v;
	vec_add_one(v);
	int   move_bytes = elem_size * ((v->_alloc - 1) - idx);
	void* pos        = vec_iter_at_(v, idx, elem_size);

	memmove((uint8_t*)pos + elem_size, pos, move_bytes);
	memcpy(pos, item, elem_size);
}

void vec_insert_at_(void* gen_v, int idx, const void* it, int n, int elem_size) {
	Vec*        v    = gen_v;
	void*       pos  = vec_iter_at_(v, idx, elem_size);
	const void* back = (const uint8_t*)it + ((n - 1) * elem_size);
	vec_insert_iter_(v, pos, it, back, elem_size);
}

void vec_insert_(void* gen_v, void* pos, const void* it, int n, int elem_size) {
	if (n == 0) {
		return;
	}
	Vec*        v    = gen_v;
	const void* back = (const uint8_t*)it + elem_size * (n - 1);
	vec_insert_iter_(v, pos, it, back, elem_size);
}

/** Deletion **/
void vec_erase_iter_(void* gen_v, void* begin, const void* back, int elem_size) {
	Vec* v     = gen_v;
	int  bytes = (const uint8_t*)vec_iter_at_(v, v->_alloc - 1, elem_size)
	            - (const uint8_t*)back;
	v->size -= _iter_size_(begin, back, elem_size);
	memmove(begin, (uint8_t*)back + elem_size, bytes);
}

void vec_erase_at_(void* gen_v, int idx, int n, int elem_size) {
	void*       begin = vec_iter_at_(gen_v, idx, elem_size);
	const void* back  = vec_iter_at_(gen_v, idx + n - 1, elem_size);
	vec_erase_iter_(gen_v, begin, back, elem_size);
}

void vec_erase_(void* gen_v, void* it, int n, int elem_size) {
	if (n == 0) {
		return;
	}
	const void* back = (char*)it + elem_size * (n - 1);
	vec_erase_iter_(gen_v, it, back, elem_size);
}

/** Appending **/
void vec_append_(void* gen_v, const void* it, int n, int elem_size) {
	Vec* v        = gen_v;
	int  old_size = v->size;
	vec_resize(v, v->size + n);
	void* end = vec_iter_at_(v, old_size, elem_size);
	memcpy(end, it, n * elem_size);
}

void vec_extend_(void* gen_v, const void* vec_src, int elem_size) {
	Vec*       v     = gen_v;
	const Vec* src   = vec_src;
	int        index = v->size;
	vec_resize(v, v->size + src->size);
	void*  end   = vec_iter_at_(v, index, elem_size);
	size_t bytes = elem_size * (src->size + 1);
	memmove(end, vec_begin(*src), bytes);
}

#ifdef __unix__
void vec_sort_r_(void* gen_v, qsort_r_cmp_fn cmp__, void* context, int elem_size) {
	Vec* v = gen_v;
	qsort_r(v->data, v->size, elem_size, cmp__, context);
}
#endif /* unix */

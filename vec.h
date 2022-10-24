#ifndef VEC_H
#define VEC_H

/**
 * This is a simple vector implementation. Due to C's lacking of
 * generics, there is lots of macro play here. functions with
 * trailing underscores take an element size parameter, and will
 * _always_ take a void* to the vector. In most cases, prefer
 * functions without trailing underscores. Their main purpose is
 * to deal with cases where type information is not available.
 */

/* This is only for qsort_r used by vec_sort_r */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define Vec(T_)                 \
	struct {                \
		T_*     data;   \
		int32_t size;   \
		int32_t _alloc; \
	}

/* Base vector */
typedef Vec(uint8_t) Vec;

#define VEC_ALLOC_DEFAULT 2

/** Utility **/
#define vec_get_idx(V_, POS_) (POS_ - vec_begin(V_))
#define vec_elem_size(V_)     sizeof(*(V_).data)

/** Create and destroy **/
#define make_vec(T_)                                                  \
	{                                                             \
		.data   = heap_alloc(sizeof(T_) * VEC_ALLOC_DEFAULT), \
		._alloc = VEC_ALLOC_DEFAULT                           \
	}
void* vec_construct_(void*, int elem_size);
#define vec_construct(V_) vec_construct_(V_, vec_elem_size(*(V_)))
#define vec_destroy(V_)                   \
	{                                 \
		if ((V_)->data != NULL) { \
			free((V_)->data); \
		}                         \
	}

void vec_clone_(void* dest, void* src, int elem_size);
#define vec_clone(DEST_, SRC_)           \
	{                                \
		vec_construct(DEST_);    \
		vec_extend(DEST_, SRC_); \
	}

void vec_copy_(void* dest, void* src, int elem_size);
#define vec_copy(DEST_, SRC_)           \
	{                                \
		vec_clear(DEST_);    \
		vec_extend(DEST_, SRC_); \
	}

/** Accessing **/
#define vec_empty(V_) ((V_).size == 0)
/* No way to do this even with element size. See vec_iter_at_. */
#define vec_at(V_, IDX_) ((V_).data[IDX_])


/** Iterators **/
void* vec_iter_at_(const void*, int idx, int elem_size);
#define vec_iter_at(V_, IDX_) (&(V_).data[IDX_])

#define vec_begin_(V_, _) vec_begin(*(V_))
#define vec_begin(V_)     (&(V_).data[0])

void* vec_back_(const void*, int elem_size);
#define vec_back(V_) (&(V_).data[vec_empty(V_) ? 0 : (V_).size - 1])

void* vec_end_(const void*, int elem_size);
#define vec_end(V_) (&(V_).data[(V_).size])


/** Resizing **/
void vec_reserve_(void*, int n, int elem_size);
#define vec_reserve(V_, ALLOC_) vec_reserve_(V_, ALLOC_, vec_elem_size(*(V_)))

void vec_resize_(void*, int n, int elem_size);
#define vec_resize(V_, N_)             \
	{                              \
		vec_reserve((V_), N_); \
		(V_)->size = N_;       \
	}

void vec_resize_and_zero_(void*, int n, int elem_size);
#define vec_resize_and_zero(V_, N_) vec_resize_and_zero_(V_, N_, vec_elem_size(*(V_)))

/** Shrinking **/
#define vec_clear(V_)    (V_)->size = 0
#define vec_pop_back(V_) (V_)->size == 0 ? NULL : &(V_)->data[--(V_)->size]

#define vec_shrink_to_fit(V_)                                               \
	{                                                                   \
		realloc_((V_)->data, ((V_)->size + 1) * vec_elem_size(V_)); \
		(V_)->_alloc = (V_)->size + 1;                              \
	}

/** Growing **/
void* vec_add_one_(void*, int elem_size);
#define vec_add_one(V_) vec_add_one_(V_, vec_elem_size(*(V_)))

void* vec_add_one_front_(void*, int elem_size);
#define vec_add_one_front(V_) vec_add_one_front_(V_, vec_elem_size(V_))

void vec_push_back_(void*, const void* item_ptr, int elem_size);
#define vec_push_back(V_, ITEM_)                    \
	{                                           \
		vec_add_one(V_);                    \
		(V_)->data[(V_)->size - 1] = ITEM_; \
	}

/** Assignment **/
#define vec_set_iter_(V_, POS_, BEGIN_, BACK_, _) vec_set_iter(V_, POS_, BEGIN_, BACK_)
#define vec_set_iter(V_, POS_, BEGIN_, BACK_) \
	memcpy(POS_, BEGIN_, (char*)BACK_ - (char*)BEGIN_ + 1);

#define vec_set_one_(V_, POS_, src_, ES_) memcpy(POS_, src_, ES_);
#define vec_set_one(V_, POS_, ITEM_)      *(POS_) = ITEM_

#define vec_set_one_at_(V_, IDX_, src_, ES_) vec_set_at_(V_, IDX_, src_, 1, ES_)
#define vec_set_one_at(V_, IDX_, ITEM_)      (V_)->data[IDX_] = ITEM_

void vec_set_at_(void*, int idx, const void* src, int n, int elem_size);
#define vec_set_at(V_, IDX_, src_, N_) \
	memcpy(&(V_)->data[IDX_], src_, N_* vec_elem_size(*(V_)))

#define vec_set_(V_, POS_, src_, N_, ES_) memcpy(POS_, src_, N_* ES_)
#define vec_set(V_, POS_, src_, N_)       memcpy(POS_, src_, N_* vec_elem_size(*(V_)))

/** Insertion **/
void vec_insert_iter_(
    void*, void* pos, const void* begin, const void* back, int elem_size);
#define vec_insert_iter(V_, POS_, BEGIN_, BACK_)                               \
	{                                                                      \
		int idx_       = vec_get_idx(*(V_), POS_);                     \
		int iter_size_ = BEGIN_ - BACK_ + 1;                           \
		vec_resize(V_, (V_)->size + iter_size_);                       \
		int move_bytes_ =                                              \
		    vec_elem_size(*(V_)) * ((V_)->_alloc - idx_ - iter_size_); \
		memmove(&POS_[iter_size_], POS_, move_bytes_);                 \
		memcpy(POS_, BEGIN_, iter_size_* vec_elem_size(*(V_)));        \
	}

void vec_insert_one_(void*, void* pos, const void* item, int elem_size);
#define vec_insert_one(V_, POS_, ITEM_)                                             \
	{                                                                           \
		int idx_ = vec_get_idx(*(V_), POS_);                                \
		vec_add_one(V_);                                                    \
		int move_bytes_ = vec_elem_size(*(V_)) * ((V_)->_alloc - 1 - idx_); \
		memmove(&POS_[1], POS_, move_bytes_);                               \
		*(POS_) = ITEM_;                                                    \
	}

void vec_insert_one_at_(void*, int idx, const void* item, int elem_size);
#define vec_insert_one_at(V_, IDX_, ITEM_) \
	vec_insert_one(V_, vec_iter_at(*(V_), IDX_), ITEM_)

void vec_insert_at_(void*, int idx, const void* it, int n, int elem_size);
#define vec_insert_at(V_, IDX_, IT_, N_) \
	vec_insert_iter(V_, vec_iter_at(*(V_), IDX_), IT_, &(IT_)[N_ - 1])

void vec_insert_(void*, void* pos, const void* it, int n, int elem_size);
#define vec_insert(V_, POS_, IT_, N_) vec_insert_iter(V_, POS_, IT_, &(IT_)[N_ - 1])

/** Deletion **/
void vec_erase_iter_(void*, void* begin, const void* back, int elem_size);
#define vec_erase_iter(V_, BEGIN_, BACK_)                                      \
	{                                                                      \
		int bytes_ = vec_elem_size(*(V_))                              \
		             * (vec_iter_at(*(V_), (V_)->_alloc - 1) - BACK_); \
		(V_)->size -= (BACK_ - BEGIN_ + 1);                            \
		memmove(BEGIN_, &(BACK_)[1], bytes_);                          \
	}

#define vec_erase_one_at_(V_, IDX_, ES_) vec_erase_one_(V_, vec_iter_at(*(V_), IDX_))
#define vec_erase_one_at(V_, IDX_)       vec_erase_one(V_, vec_iter_at(*(V_), IDX_))

#define vec_erase_one_(V_, IT_, ES_) vec_erase_iter_(V_, IT_, IT_, ES_)
#define vec_erase_one(V_, IT_)       vec_erase_iter(V_, IT_, IT_)

void vec_erase_at_(void*, int idx, int n, int elem_size);
#define vec_erase_at(V_, IDX_, N_)                             \
	{                                                      \
		if (N_) {                                      \
			vec_erase_iter(V_,                     \
			    vec_iter_at(*(V_), IDX_),          \
			    vec_iter_at(*(V_), IDX_ + N_ - 1)) \
		}                                              \
	}

void vec_erase_(void*, void* it, int n, int elem_size);
#define vec_erase(V_, IT_, N_)                                \
	{                                                     \
		if (N_) {                                     \
			vec_erase_iter(V_, IT_, IT_ + N_ - 1) \
		}                                             \
	}

/** Appending **/
void vec_append_(void*, const void* it, int n, int elem_size);
#define vec_append(V_, IT_, N_)                                                  \
	{                                                                        \
		int idx_ = (V_)->size;                                           \
		vec_resize(V_, idx_ + N_);                                       \
		memcpy(vec_iter_at(*(V_), idx_), IT_, N_* vec_elem_size(*(V_))); \
	}

void vec_extend_(void*, const void* vec_src, int elem_size);
#define vec_extend(v_dest_, v_src_)                                               \
	{                                                                         \
		int idx_ = (v_dest_)->size;                                       \
		vec_resize(v_dest_, (v_dest_)->size + (v_src_).size);             \
		int bytes = vec_elem_size(v_src_) * ((v_src_).size + 1);          \
		memmove(vec_iter_at(*(v_dest_), idx_), vec_begin(v_src_), bytes); \
	}


/** Sort **/
typedef int (*qsort_r_cmp_fn)(const void*, const void*, void*);

#ifdef __unix__
void vec_sort_r_(void*, qsort_r_cmp_fn, void* context, int elem_size);
#define vec_sort_r(V_, fn_, context_) \
	qsort_r((V_)->data, (V_)->size, vec_elem_size(V_), fn_, context_)
#endif /* unix */

#endif /* VEC_H */

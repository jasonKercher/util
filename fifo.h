#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "vec.h"

#if __STDC_VERSION__ < 201112L
#define ATOMIC_
#else
#include <stdatomic.h>
#define ATOMIC_ _Atomic
#endif

/**
 * naive thread-safe circular buffer
 */

#define Fifo(T_)                                  \
	struct {                                  \
		Vec(T_) buf;                      \
		ATOMIC_(void*) shared_mutex_fifo; \
		pthread_mutex_t head_mutex;       \
		pthread_mutex_t tail_mutex;       \
		pthread_cond_t cond_add;          \
		pthread_cond_t cond_get;          \
		unsigned input_count;             \
		ATOMIC_ unsigned head;            \
		ATOMIC_ unsigned tail;            \
		ATOMIC_ unsigned _iter_head;      \
		ATOMIC_ bool is_open;             \
	}

typedef Fifo(uint8_t) Fifo;

void* fifo_construct_(void*, unsigned buf_size, int elem_size);
#define fifo_construct(f_, n_) fifo_construct_(f_, n_, sizeof(*(f_)->buf.data))
void fifo_free(void*);
void fifo_destroy(void*);
void fifo_reset(void*);

#define fifo_resize(f_, n_)                          \
	{                                            \
		if (n_ <= 1) {                       \
			n_ = 2;                      \
		}                                    \
		(f_)->head = 0;                      \
		(f_)->tail = 0;                      \
		vec_resize_and_zero(&(f_)->buf, n_); \
	}

void fifo_set_open(void*, int);
unsigned fifo_available(const void*);

#define fifo_is_empty(f_)   ((f_)->head == (f_)->tail)
#define fifo_is_full(f_)    (((f_)->head + 1) % (f_)->buf.size == (f_)->tail)
#define fifo_receivable(f_) (f_)->buf.size - fifo_available(f_) - (f_)->input_count
#define fifo_set_full(f_)                        \
	{                                        \
		(f_)->tail = 0;                  \
		(f_)->head = (f_)->buf.size - 1; \
	}

#define fifo_peek(f_) &vec_at((f_)->buf, (f_)->tail)

void* fifo_get_or_wait_(void*, int);
#define fifo_get_or_wait(f_) fifo_get_or_wait_(f_, vec_elem_size((f_)->buf))

void* fifo_get_(void*, int);
#define fifo_get(f_) fifo_get_(f_, vec_elem_size((f_)->buf))

int fifo_nget_(
        void*, void* restrict buf, unsigned block_size, unsigned max, int elem_size);
#define fifo_nget(f_, buf_, block_size_, max_) \
	fifo_nget_(f_, buf_, block_size_, max_, vec_elem_size((f_)->buf)

void* fifo_safe_peek_(void*, int elem_size);
#define fifo_safe_peek(f_) fifo_safe_peek(f_, vec_elem_size((f_)->buf))

void* fifo_look_ahead_(const void*, int elem_size);
#define fifo_look_ahead(f_) fifo_look_ahead_(f_, vec_elem_size((f_)->buf))

void fifo_consume(void*);

void fifo_add_(void*, void* restrict, int iter_size);
#define fifo_add(f_, item_)                                     \
	{                                                       \
		pthread_mutex_lock(&(f_)->head_mutex);          \
		vec_set_one_at(&(f_)->buf, (f_)->head, item_);  \
		(f_)->head = ((f_)->head + 1) % (f_)->buf.size; \
		pthread_cond_signal(&(f_)->cond_add);           \
		fifo_signal_shared(f_);                         \
		pthread_mutex_unlock(&(f_)->head_mutex);        \
	}

//int fifo_add_try(void*, void* restrict);
void fifo_nadd_(void*, Vec*, int elem_size);
#define fifo_nadd(f_, buf_) fifo_nadd_(f_, (Vec*)(buf_), vec_elem_size((f_)->buf))
//int fifo_nadd_try(void*, Vec* restrict);
void fifo_advance(void*);


/* these iterators do not touch mutexes
 * and do not send signals. Up to user
 * to call update() after done iterating.
 */
void* fifo_begin_(void*, int elem_size);
#define fifo_begin(f_) fifo_begin_(f_, vec_elem_size((f_)->buf))

void* fifo_iter_(void*, int elem_size);
#define fifo_iter(f_) fifo_iter_(f_, vec_elem_size((f_)->buf))

#define fifo_end(f_) vec_at((f_)->buf, (f_)->_iter_head)

void fifo_update(void*);
int fifo_update_try(void*);

/* thread conditions */
void fifo_wait_for_add(void*);
void fifo_wait_for_get(void*);
void fifo_wait_for_add_either(void*, void*);
void fifo_wait_for_add_both(void*, void*);
bool fifo_signal_shared(void*);

#endif /* FIFO_H */

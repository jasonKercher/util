#ifndef QUEUE_H
#define QUEUE_H

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

#define Queue(T_)                                  \
	struct {                                   \
		Vec(T_) buf;                       \
		ATOMIC_(void*) shared_mutex_queue; \
		pthread_mutex_t  head_mutex;       \
		pthread_mutex_t  tail_mutex;       \
		pthread_cond_t   cond_add;         \
		pthread_cond_t   cond_get;         \
		unsigned         input_count;      \
		ATOMIC_ unsigned head;             \
		ATOMIC_ unsigned tail;             \
		ATOMIC_ unsigned _iter_head;       \
		ATOMIC_ bool     is_open;          \
	}

typedef Queue(uint8_t) Queue;

void* queue_construct_(void*, unsigned buf_size, int elem_size);
#define queue_construct(f_, n_) queue_construct_(f_, n_, sizeof(*(f_)->buf.data))
void queue_free(void*);
void queue_destroy(void*);
void queue_reset(void*);

#define queue_resize(f_, n_)                         \
	{                                            \
		if (n_ <= 1) {                       \
			n_ = 2;                      \
		}                                    \
		(f_)->head = 0;                      \
		(f_)->tail = 0;                      \
		vec_resize_and_zero(&(f_)->buf, n_); \
	}

void     queue_set_open(void*, int);
unsigned queue_available(const void*);

#define queue_is_empty(f_)   ((f_)->head == (f_)->tail)
#define queue_is_full(f_)    (((f_)->head + 1) % (f_)->buf.len == (f_)->tail)
#define queue_receivable(f_) (f_)->buf.len - queue_available(f_) - (f_)->input_count
#define queue_set_full(f_)                      \
	{                                       \
		(f_)->tail = 0;                 \
		(f_)->head = (f_)->buf.len - 1; \
	}

#define queue_peek(f_) &vec_at((f_)->buf, (f_)->tail)

void* queue_get_or_wait_(void*, int);
#define queue_get_or_wait(f_) queue_get_or_wait_(f_, vec_elem_size((f_)->buf))

void* queue_get_(void*, int);
#define queue_get(f_) queue_get_(f_, vec_elem_size((f_)->buf))

int queue_nget_(
    void*, void* restrict buf, unsigned block_size, unsigned max, int elem_size);
#define queue_nget(f_, buf_, block_size_, max_) \
	queue_nget_(f_, buf_, block_size_, max_, vec_elem_size((f_)->buf)

void* queue_safe_peek_(void*, int elem_size);
#define queue_safe_peek(f_) queue_safe_peek(f_, vec_elem_size((f_)->buf))

void* queue_look_ahead_(const void*, int elem_size);
#define queue_look_ahead(f_) queue_look_ahead_(f_, vec_elem_size((f_)->buf))

void queue_consume(void*);

void queue_add_(void*, void* restrict, int iter_size);
#define queue_add(f_, item_)                                   \
	{                                                      \
		pthread_mutex_lock(&(f_)->head_mutex);         \
		vec_set_one_at(&(f_)->buf, (f_)->head, item_); \
		(f_)->head = ((f_)->head + 1) % (f_)->buf.len; \
		pthread_cond_signal(&(f_)->cond_add);          \
		queue_signal_shared(f_);                       \
		pthread_mutex_unlock(&(f_)->head_mutex);       \
	}

//int queue_add_try(void*, void* restrict);
void queue_nadd_(void*, Vec*, int elem_size);
#define queue_nadd(f_, buf_) queue_nadd_(f_, (Vec*)(buf_), vec_elem_size((f_)->buf))
//int queue_nadd_try(void*, Vec* restrict);
void queue_advance(void*);


/* these iterators do not touch mutexes
 * and do not send signals. Up to user
 * to call update() after done iterating.
 */
void* queue_begin_(void*, int elem_size);
#define queue_begin(f_) queue_begin_(f_, vec_elem_size((f_)->buf))

void* queue_iter_(void*, int elem_size);
#define queue_iter(f_) queue_iter_(f_, vec_elem_size((f_)->buf))

#define queue_end(f_) vec_at((f_)->buf, (f_)->_iter_head)

void queue_update(void*);
int  queue_update_try(void*);

/* thread conditions */
void queue_wait_for_add(void*);
void queue_wait_for_get(void*);
void queue_wait_for_add_either(void*, void*);
void queue_wait_for_add_both(void*, void*);
bool queue_signal_shared(void*);

#endif /* QUEUE_H */

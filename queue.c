#include "queue.h"
#include "util.h"

#define _idx_adv_(idx_) idx_ = (idx_ + 1) % f->buf.len;
#define _queue_peek()    f->buf.data + elem_size * f->tail;


void* queue_construct_(void* gen_f, unsigned buf_size, int elem_size)
{
	Queue* f = gen_f;
	/* Queue requires a buffer of atleast size 2 */
	if (buf_size <= 1) {
		buf_size = 2;
	}
	memset(f, 0, sizeof(*f));
	vec_construct_(&f->buf, elem_size);
	f->is_open = true;
	f->shared_mutex_queue = NULL;

	vec_resize_and_zero(&f->buf, buf_size);

	pthread_mutex_init(&f->head_mutex, NULL);
	pthread_mutex_init(&f->tail_mutex, NULL);
	pthread_cond_init(&f->cond_add, NULL);
	pthread_cond_init(&f->cond_get, NULL);

	return f;
}

void queue_destroy(void* gen_f)
{
	Queue* f = gen_f;
	vec_destroy(&f->buf);
	pthread_mutex_destroy(&f->head_mutex);
	pthread_mutex_destroy(&f->tail_mutex);
	pthread_cond_destroy(&f->cond_add);
	pthread_cond_destroy(&f->cond_get);
}

void queue_free(void* f)
{
	queue_destroy(f);
	heap_free(f);
}

void queue_reset(void* gen_f)
{
	Queue* f = gen_f;
	if (f == NULL) {
		return;
	}

	f->is_open = true;
	f->head = 0;
	f->tail = 0;
	f->_iter_head = 0;
}

/* this struct was never meant to be resizable
 * but under the assumption that you are okay
 * losing all currently held data... yea...
 * go for it. (definitely not thread-safe)
 */
void queue_set_open(void* gen_f, int is_open)
{
	Queue* restrict f = gen_f;
	pthread_mutex_lock(&f->tail_mutex);
	pthread_mutex_lock(&f->head_mutex);

	f->is_open = is_open;

	pthread_cond_broadcast(&f->cond_get);
	pthread_cond_broadcast(&f->cond_add);
	if (f->shared_mutex_queue != NULL) {
		Queue* restrict shared_queue = f->shared_mutex_queue;
		pthread_mutex_lock(&shared_queue->head_mutex);
		pthread_cond_broadcast(&shared_queue->cond_add);
		pthread_mutex_unlock(&shared_queue->head_mutex);
		f->shared_mutex_queue = NULL;
	}

	pthread_mutex_unlock(&f->head_mutex);
	pthread_mutex_unlock(&f->tail_mutex);
}

unsigned queue_available(const void* gen_f)
{
	const Queue* f = gen_f;
	unsigned available = f->head - f->tail;
	if (available > (unsigned)f->buf.len) {
		available += f->buf.len;
	}
	return available;
}

void* queue_get_or_wait_(void* gen_f, int elem_size)
{
	Queue* f = gen_f;
	pthread_mutex_lock(&f->tail_mutex);
	while (!queue_available(f)) {
		queue_wait_for_add(f);
	}
	void* data = _queue_peek();
	_idx_adv_(f->tail);
	pthread_cond_signal(&f->cond_get);
	pthread_mutex_unlock(&f->tail_mutex);
	return data;
}

void* queue_get_(void* gen_f, int elem_size)
{
	Queue* f = gen_f;
	pthread_mutex_lock(&f->tail_mutex);
	void* data = _queue_peek();
	_idx_adv_(f->tail);
	pthread_cond_signal(&f->cond_get);
	pthread_mutex_unlock(&f->tail_mutex);
	return data;
}

int queue_nget_(void* gen_f,
               void* restrict buffer,
               unsigned block_size,
               unsigned max,
               int elem_size)
{
	Queue* f = gen_f;
	pthread_mutex_lock(&f->tail_mutex);
	unsigned available = queue_available(f);
	if (available == 0) {
		pthread_mutex_unlock(&f->tail_mutex);
		return 0;
	}
	unsigned transfer_count = (available > max) ? max : available;
	transfer_count -= transfer_count % block_size;
	unsigned new_tail = (f->tail + transfer_count) % f->buf.len;

	if (new_tail == 0 || new_tail > f->tail) {
		//vec_append(buffer, vec_at(f->buf, f->tail), new_tail - f->tail);
		vec_append_(buffer, &vec_at(f->buf, f->tail), transfer_count, elem_size);
	} else {
		vec_append_(buffer,
		            &vec_at(f->buf, f->tail),
		            f->buf.len - f->tail,
		            elem_size);
		vec_append_(buffer, vec_begin(f->buf), new_tail, elem_size);
	}

	f->tail = new_tail;

	pthread_cond_signal(&f->cond_get);
	pthread_mutex_unlock(&f->tail_mutex);

	/* Return what we *know* is available */
	return available - transfer_count;
}

void* queue_safe_peek_(void* gen_f, int elem_size)
{
	Queue* f = gen_f;
	void* data = NULL;

	pthread_mutex_lock(&f->tail_mutex);
	if (f->head != f->tail) {
		data = vec_iter_at_(&f->buf, f->tail, elem_size);
	}
	pthread_mutex_unlock(&f->tail_mutex);

	return data;
}

void* queue_look_ahead_(const void* gen_f, int elem_size)
{
	const Queue* f = gen_f;
	if (queue_available(f) < 2) {
		return NULL;
	}
	unsigned look_ahead_idx = f->tail;
	_idx_adv_(look_ahead_idx);

	return vec_iter_at_(&f->buf, look_ahead_idx, elem_size);
}

void queue_consume(void* gen_f)
{
	Queue* f = gen_f;
	pthread_mutex_lock(&f->tail_mutex);
	_idx_adv_(f->tail);
	pthread_cond_signal(&f->cond_get);
	pthread_mutex_unlock(&f->tail_mutex);
}

//int queue_add_try(void* gen_f, void* restrict data)
//{
//	Queue* f = gen_f;
//	int ret = pthread_mutex_trylock(&f->head_mutex);
//	if (ret != 0) {
//		return ret;
//	}
//	vec_set_at(f->buf, f->head, data, 1);
//	_idx_adv_(f->head);
//	pthread_cond_signal(&f->cond_add);
//	queue_signal_shared(f);
//	pthread_mutex_unlock(&f->head_mutex);
//	return 0;
//}

void queue_nadd_(void* gen_f, Vec* src, int elem_size)
{
	Queue* f = gen_f;
	pthread_mutex_lock(&f->head_mutex);

	int receivable = queue_receivable(f);
	if (receivable == 0) {
		pthread_mutex_unlock(&f->head_mutex);
		return;
	}

	int transfer_count = (receivable < src->len) ? receivable : src->len;
	unsigned new_head = (f->head + transfer_count) % f->buf.len;

	if (new_head == 0 || new_head > f->head) {
		vec_set_at_(&f->buf, f->head, vec_begin(*src), transfer_count, elem_size);
	} else {
		unsigned transfer1 = f->buf.len - f->head;
		vec_set_at_(&f->buf, f->head, vec_begin(*src), transfer1, elem_size);
		vec_set_at_(&f->buf,
		            0,
		            vec_iter_at_(src, transfer1, elem_size),
		            transfer_count - transfer1,
		            elem_size);
	}

	f->head = new_head;

	pthread_cond_signal(&f->cond_add);
	queue_signal_shared(f);
	pthread_mutex_unlock(&f->head_mutex);

	if (transfer_count == src->len) {
		vec_clear(src);
	} else {
		vec_erase(src, vec_begin(*src), transfer_count);
	}
}

//int queue_nadd_try(void* gen_f, vec* restrict src)
//{
//	Queue* f = gen_f;
//	int ret = pthread_mutex_trylock(&f->head_mutex);
//	if (ret != 0) {
//		return ret;
//	}
//	unsigned receivable = queue_receivable(f);
//	if (receivable == 0) {
//		pthread_mutex_unlock(&f->head_mutex);
//		return 0;
//	}
//
//	unsigned transfer_count = (receivable < src->len) ? receivable : src->len;
//	unsigned new_head = (f->head + transfer_count) % f->buf.len;
//
//	if (new_head == 0 || new_head > f->head) {
//		vec_set_at(f->buf, f->head, vec_begin(src), transfer_count);
//	} else {
//		unsigned transfer1 = f->buf.len - f->head;
//		vec_set_at(f->buf, f->head, vec_begin(src), transfer1);
//		vec_set_at(f->buf, 0, vec_at(src, transfer1), transfer_count - transfer1);
//	}
//
//	f->head = new_head;
//
//	pthread_cond_signal(&f->cond_add);
//	queue_signal_shared(f);
//	pthread_mutex_unlock(&f->head_mutex);
//
//	if (transfer_count == src->len) {
//		vec_clear(src);
//	} else {
//		vec_erase(src, vec_begin(src), transfer_count);
//	}
//
//	return 0;
//}

void queue_advance(void* gen_f)
{
	Queue* f = gen_f;
	pthread_mutex_lock(&f->head_mutex);
	_idx_adv_(f->head);
	pthread_cond_signal(&f->cond_add);
	pthread_mutex_unlock(&f->head_mutex);
}


void* queue_begin_(void* gen_f, int elem_size)
{
	Queue* f = gen_f;
	f->_iter_head = f->head % f->buf.len;
	return _queue_peek();
}

void* queue_iter_(void* gen_f, int elem_size)
{
	Queue* f = gen_f;
	/* consume without mutexes */
	_idx_adv_(f->tail);
	return _queue_peek();
}

void queue_update(void* gen_f)
{
	Queue* f = gen_f;
	pthread_mutex_lock(&f->tail_mutex);
	pthread_cond_signal(&f->cond_get);
	pthread_mutex_unlock(&f->tail_mutex);
}

int queue_update_try(void* gen_f)
{
	Queue* f = gen_f;
	int ret = pthread_mutex_trylock(&f->tail_mutex);
	if (ret != 0) {
		return ret;
	}
	pthread_cond_signal(&f->cond_get);
	pthread_mutex_unlock(&f->tail_mutex);
	return 0;
}


void queue_wait_for_add(void* gen_f)
{
	Queue* f = gen_f;
	pthread_mutex_lock(&f->head_mutex);
	while (f->is_open && queue_is_empty(f)) {
		pthread_cond_wait(&f->cond_add, &f->head_mutex);
	}
	pthread_mutex_unlock(&f->head_mutex);
}

void queue_wait_for_add_either(void* gen_f0, void* gen_f1)
{
	Queue* restrict f0 = gen_f0;
	Queue* restrict f1 = gen_f1;
	//pthread_mutex_lock(&f0->head_mutex);
	pthread_mutex_lock(&f1->head_mutex);
	while (f1->is_open && queue_is_empty(f1)) {
		pthread_mutex_lock(&f0->head_mutex);
		if (!f0->is_open || !queue_is_empty(f0)) {
			pthread_mutex_unlock(&f0->head_mutex);
			break;
		}
		f0->shared_mutex_queue = f1;
		pthread_mutex_unlock(&f0->head_mutex);
		pthread_cond_wait(&f1->cond_add, &f1->head_mutex);
		f0->shared_mutex_queue = NULL;
	}
	pthread_mutex_unlock(&f1->head_mutex);
	//pthread_mutex_unlock(&f0->head_mutex);
}

void queue_wait_for_add_both(void* gen_f0, void* gen_f1)
{
	Queue* restrict f0 = gen_f0;
	Queue* restrict f1 = gen_f1;
	while (f1->is_open && f0->is_open && (queue_is_empty(f0) || queue_is_empty(f1))) {
		queue_wait_for_add(f0);
		queue_wait_for_add(f1);
	}
}

void queue_wait_for_get(void* gen_f)
{
	Queue* f = gen_f;
	pthread_mutex_lock(&f->tail_mutex);
	while (!queue_receivable(f)) {
		pthread_cond_wait(&f->cond_get, &f->tail_mutex);
	}
	pthread_mutex_unlock(&f->tail_mutex);
}

bool queue_signal_shared(void* gen_f)
{
	Queue* f = gen_f;
	Queue* shared = f->shared_mutex_queue;
	if (shared == NULL) {
		return false;
	}
	pthread_mutex_lock(&shared->head_mutex);
	pthread_cond_signal(&shared->cond_add);
	pthread_mutex_unlock(&shared->head_mutex);
	f->shared_mutex_queue = NULL;
	return true;
}


/* dumbed down nadds */

//void queue_nadd(void* gen_f, vec* restrict src)
//{
//	pthread_mutex_lock(&f->head_mutex);
//
//	unsigned receivable = queue_receivable(f);
//	unsigned transfer_count = (receivable < src->len) ? receivable : src->len;
//	unsigned i = 0;
//	for (; i < transfer_count; ++i) {
//		vec_set_at(f->buf, f->head, vec_at(f->buf, i), 1);
//		_idx_adv_(f->head);
//	}
//
//	pthread_cond_signal(&f->cond_add);
//	queue_signal_shared(f);
//	pthread_mutex_unlock(&f->head_mutex);
//
//	if (i == src->len) {
//		vec_clear(src);
//	} else {
//		vec_erase(src, vec_begin(src), i);
//	}
//}
//
//int queue_nadd_try(void* gen_f, vec* restrict src)
//{
//	int ret = pthread_mutex_trylock(&f->head_mutex);
//	if (ret != 0) {
//		return ret;
//	}
//	unsigned receivable = queue_receivable(f);
//	unsigned transfer_count = (receivable < src->len) ? receivable : src->len;
//	unsigned i = 0;
//	for (; i < transfer_count; ++i) {
//		vec_set_at(f->buf, f->head, vec_at(f->buf, i), 1);
//		_idx_adv_(f->head);
//	}
//
//	pthread_cond_signal(&f->cond_add);
//	queue_signal_shared(f);
//	pthread_mutex_unlock(&f->head_mutex);
//
//	if (i == src->len) {
//		vec_clear(src);
//	} else {
//		vec_erase(src, vec_begin(src), i);
//	}
//
//	return 0;
//}

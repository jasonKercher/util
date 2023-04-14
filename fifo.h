#ifndef FIFO_H
#define FIFO_H

#include <stdatomic.h>
#include "types.h"

/* NOTE: This header uses "statement expresions"
 *       and typeof which are GNU extensions.
 */

/* Use anonymous struct or typedef */
#define Fifo(T_, LEN_)                 \
	struct {                       \
		_Atomic u16 head;      \
		_Atomic u16 tail;      \
		T_          buf[LEN_]; \
	}

#define fifo_len(F_)                 (sizeof((F_).buf) / sizeof((F_).buf[0]))
#define FIFO_IDX_ADD(F_, IDX_, OFF_) IDX_ = (IDX_ + (OFF_)) % fifo_len(F_)

/* access: These "functions" operate directly on the struct
 *         because there is no modification.
 */
#define fifo_available(F_)                           \
	({                                           \
		uint avail_ = (F_).head - (F_).tail; \
		if (avail_ > fifo_len(F_)) {         \
			avail_ += fifo_len(F_);      \
		}                                    \
		avail_;                              \
	})

#define fifo_is_empty(F_) ((F_).head == (F_).tail)
#define fifo_is_full(F_)  (((F_).head + 1) % fifo_len(F_) == (F_).tail)

#define fifo_peek(F_)              ((F_).buf[(F_).tail])
#define fifo_look_ahead(F_)        ((F_).buf[((F_).tail + 1) % fifo_len(F_)])
#define fifo_idx_is_sane(F_, IDX_) ((F_).head - (F_).tail >= (IDX_) - (F_).tail)

/* mutate: These "functions" take a pointer to the struct in order
 *         to signal to the user there are mutations here.
 */
#define fifo_clear(F_) (F_)->tail = (F_)->head

#define fifo_consume(F_, OFF_) FIFO_IDX_ADD(*(F_), (F_)->tail, OFF_)
#define fifo_get(F_)                                               \
	({                                                         \
		typeof((F_)->buf[0]) res_ = (F_)->buf[(F_)->tail]; \
		fifo_consume(F_, 1);                               \
		res_;                                              \
	})

#define fifo_advance(F_, OFF_) FIFO_IDX_ADD(*(F_), (F_)->head, OFF_)
#define fifo_add(F_, ITEM_)                    \
	{                                      \
		(F_)->buf[(F_)->head] = ITEM_; \
		fifo_advance(F_, 1);           \
	}

#endif /* FIFO_H */

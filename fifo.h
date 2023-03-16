/* ========================================
 * project: RLC294 -- Topcon Medical Systems 2200/2300/2400 Chair Controller
 * File:    Fifo.h
 *
 * First In First Out circular buffer
 *
 * Rick Kundrat & Zach Martin & Jason Kercher
 * Initial version: 6/7/2022, JASON KERCHER
 *
 * Copyright RLC Electronic Systems, 2022
 * All Rights Reserved
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 *
 * ========================================
*/

#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>


/* Use anonymous struct or typedef
 * NOTE: set LEN_ to a power of 2
 */
#define Fifo(T_, LEN_)              \
	struct {                    \
		uint16_t head;      \
		uint16_t tail;      \
		T_       buf[LEN_]; \
	}

typedef Fifo(uint8_t, 64) Comms_Fifo;

#define fifo_len(F_)                 (sizeof((F_).buf) / sizeof((F_).buf[0]))
#define FIFO_IDX_ADD(F_, IDX_, OFF_) IDX_ = (IDX_ + (OFF_)) % fifo_len(F_)

/* access: These "functions" operate directly on the struct 
 *         because there is no modification.
 */
#define fifo_available(F_)                               \
	({                                               \
		unsigned avail_ = (F_).head - (F_).tail; \
		if (avail_ > fifo_len(F_)) {             \
			avail_ += fifo_len(F_);          \
		}                                        \
		avail_;                                  \
	})
#define fifo_is_empty(F_) ((F_).head == (F_).tail)
#define fifo_is_full(F_)  (((F_).head + 1) % fifo_len(F_) == (F_).tail)

#define fifo_peek(F_)              ((F_).buf[(F_).tail])
#define fifo_look_ahead(F_)        ((F_).buf[((F_).tail + 1) % fifo_len(F_)])
#define fifo_idx_is_sane(F_, IDX_) ((F_).head - (F_).tail >= (IDX_) - (F_).tail)

/* mutate: These "functions" take a pointer to the struct in order
 *         to signal to the user there are mutations here.
 */
#define fifo_clear(F_) (F_)->head = (F_)->tail

#define fifo_consume(F_, OFF_) FIFO_IDX_ADD(*(F_), (F_)->tail, OFF_)
#define fifo_get(F_)                        \
	({                                  \
		unsigned old_ = (F_)->tail; \
		fifo_consume(F_, 1);        \
		(F_)->buf[old_];            \
	})

#define fifo_advance(F_, OFF_) FIFO_IDX_ADD(*(F_), (F_)->head, OFF_)
#define fifo_add(F_, ITEM_)                    \
	{                                      \
		if (fifo_is_full(*(F_))) {     \
			fifo_consume(F_, 1);   \
		}                              \
		(F_)->buf[(F_)->head] = ITEM_; \
		fifo_advance(F_, 1);           \
	}
#define fifo_add_array(F_, ARR_, LEN_)                     \
	{                                                  \
		for (int i_ = 0; i_ < (int)(LEN_); ++i_) { \
			fifo_add(F_, (ARR_)[i_]);          \
		}                                          \
	}

#endif /* FIFO_H */

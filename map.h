#ifndef MAP_H
#define MAP_H

#include <stdlib.h>
#include <stdint.h>
#include "slice.h"
#include "vec.h"

#define MAP_PROP_DEFAULT 0x00
#define MAP_PROP_NOCASE  0x01
#define MAP_PROP_RTRIM   0x02

#define _NONE ((uint32_t)-1)

struct _Entry {
	uint64_t hash;    /* store the calculated hash for resize */
	uint64_t key_idx; /* index to start of key */
	uint32_t val_idx; /* index for _entries */
	uint32_t key_len;
};
typedef struct _Entry _Entry;
typedef Slice(_Entry) _Entry_Slice;

typedef uint64_t (*hash_fn)(uint8_t* keybuf, const char* key, unsigned* n);

struct Set {
	_Entry_Slice _entries;
	hash_fn hash__;
	Byte_Slice _keybuf;
	size_t _keybuf_head;
	unsigned map_size;
};
typedef struct Set Set;

#define Map(T_)                    \
	struct {                       \
		Vec(T_) values;        \
		hash_fn hash__;        \
		_Entry_Slice _entries; \
		Byte_Slice _keybuf;    \
		size_t _keybuf_head;   \
	}
typedef Map(uint8_t) Map;

void _map_grow_entries(_Entry_Slice* old_entries);

void set_construct(Set* restrict, size_t limit, const unsigned props);
void set_destroy(Set* restrict);
void set_clear(Set* restrict);
void set_nadd(Set* restrict, const char* restrict key, unsigned len);
#define set_add(S_, KEY_) set_nadd(S_, KEY_, strlen(KEY_))
bool set_nhas(Set* restrict, const char* restrict key, unsigned len);
#define set_has(S_, KEY_) set_nhas(S_, KEY_, strlen(KEY_))

void map_construct_(void*, const unsigned elem_size, size_t limit, const unsigned props);
#define map_construct(H_, LIMIT_, PROPS_) \
	map_construct_(H_, vec_elem_size((H_)->values), LIMIT_, PROPS_)
void map_destroy(void*);
void map_clear(void*);

/**
 * declare key into map without adding data. This is
 * just a helper function. You should not call it.
 * Returns idx or _NONE of sent key.
 */
uint32_t _map_declare(void*, const char* key, unsigned key_len);

/**
 * Add key + data pair to map
 */
uint32_t map_nset_(void*, const char* key, unsigned key_len, const void* data, int elem_size);
#define map_nset(M_, KEY_, KL_, ITEM_)                              \
	{                                                           \
		uint32_t idx_ = _map_declare(M_, KEY_, KL_);        \
		if (idx_ == _NONE) {                                \
			vec_push_back(&(M_)->values, ITEM_);        \
		} else {                                            \
			vec_set_one_at(&(M_)->values, idx_, ITEM_); \
		}                                                   \
	}
#define map_set(M_, KEY_, ITEM_) map_nset(M_, KEY_, strlen(KEY_), ITEM_)

/**
 * Return NULL if no match or pointer to value
 */
void* map_nget_(void*, const char* key, unsigned, unsigned elem_size);
#define map_nget(M_, KEY_, KL_) map_nget_(M_, KEY_, KL_, vec_elem_size((M_)->values))
#define map_get(M_, KEY_)       map_nget_(M_, KEY_, strlen(KEY_), vec_elem_size((M_)->values))

/** TODO **/
#if 0
typedef struct Map multimap;

multimap* multimap_construct(multimap* restrict,
                             const unsigned elem_size,
                             size_t limit,
                             const unsigned props);
#define multimap_construct_(h_, T_, limit_, props_) \
	multimap_construct(h_, sizeof(T_), limit_, props_)
void multimap_destroy(multimap* restrict);
void multimap_clear(multimap* restrict);
void multimap_nset(multimap* restrict,
                   const char* restrict key,
                   void* restrict,
                   unsigned);
#define multimap_set(m_, key_, data_) multimap_nset(m_, key_, data_, strlen(key_))
#define multimap_nget(m_, key_, n_)   map_nget(m_, key_, n_)
#define multimap_get(m_, key_)        map_nget(m_, key_, strlen(key_))

typedef struct Map compositemap;

compositemap* compositemap_construct(compositemap* restrict,
                                     const unsigned elem_size,
                                     size_t limit,
                                     const unsigned props);
#define compositemap_construct_(h_, T_, limit_, props_) \
	compositemap_construct(h_, sizeof(T_), limit_, props_)
void compositemap_destroy(compositemap* restrict);
void compositemap_clear(compositemap* restrict);
void compositemap_set(compositemap* restrict,
                      const struct vec* restrict key,
                      void* restrict);
void* compositemap_get(compositemap*, const struct vec* restrict key);

#endif /* 0 */

#endif /* MAP_H */

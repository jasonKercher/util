#include "map.h"
#include <string.h>
#include <ctype.h>

/* hashing based on FNV-1 */
const uint64_t _FNV1_INIT = 14695981039346656037UL;
const uint64_t _PRIME     = 1099511628211UL;

const double _FULL_PERCENT = .9;

unsigned long _next_power_of_2(unsigned long n);

_Entry* _get_entry(_Entry_Slice* entries,
    Byte_Slice*                  keybuf,
    hash_fn                      hash__,
    size_t                       keybuf_head,
    const char*                  key,
    unsigned*                    key_len,
    uint64_t*                    hash);

/* Hash functions */
uint64_t _hash(uint8_t* keybuf, const char* restrict key, unsigned* n);
uint64_t _hash_nocase(uint8_t* keybuf, const char* restrict key, unsigned* n);
uint64_t _hash_rtrim(uint8_t* keybuf, const char* restrict key, unsigned* n);
uint64_t _hash_nocase_rtrim(uint8_t* keybuf, const char* restrict key, unsigned* n);


void set_construct(Set* restrict s, size_t start_size, const unsigned props) {
	start_size = _next_power_of_2(start_size);
	*s         = (Set) {
            ._entries = slice_new(_Entry, start_size),
            ._keybuf  = slice_new(char, start_size),
        };

	switch (props) {
	case MAP_PROP_NOCASE:
		s->hash__ = _hash_nocase;
		break;
	case MAP_PROP_RTRIM:
		s->hash__ = _hash_rtrim;
		break;
	case MAP_PROP_NOCASE | MAP_PROP_RTRIM:
		s->hash__ = _hash_nocase_rtrim;
		break;
	default:
		s->hash__ = _hash;
	}

	memset(s->_entries.data, -1, sizeof(_Entry) * start_size);
}

void set_destroy(Set* restrict s) {
	HEAP_FREE(s->_entries.data);
	HEAP_FREE(s->_keybuf.data);
}

void set_clear(Set* restrict s) {
	s->_keybuf_head = 0;
	memset(s->_entries.data, -1, sizeof(_Entry) * s->_entries.len);
}

void set_nadd(Set* restrict s, const char* restrict key, unsigned n) {
	uint64_t hash = 0;
	_Entry*  e =
	    _get_entry(&s->_entries, &s->_keybuf, s->hash__, s->_keybuf_head, key, &n, &hash);

	if (e->val_idx != _NONE) {
		return;
	}

	/* new value */
	e->val_idx = 0;
	e->key_idx = s->_keybuf_head;
	e->key_len = n;
	e->hash    = hash;
	s->_keybuf_head += n;
	if (++s->map_size > _FULL_PERCENT * s->_entries.len) {
		_map_grow_entries(&s->_entries);
	}
}

bool set_nhas(Set* restrict s, const char* restrict key, unsigned n) {
	uint64_t hash = 0;
	_Entry*  entry =
	    _get_entry(&s->_entries, &s->_keybuf, s->hash__, s->_keybuf_head, key, &n, &hash);
	return (entry->val_idx != _NONE);
}

void map_construct_(
    void* gen_m, const unsigned elem_size, size_t start_size, const unsigned props) {
	Map* m     = gen_m;
	start_size = _next_power_of_2(start_size);
	*m         = (Map) {
            ._entries = slice_new(_Entry, start_size),
            ._keybuf  = slice_new(char, start_size),
        };

	switch (props) {
	case MAP_PROP_NOCASE:
		m->hash__ = _hash_nocase;
		break;
	case MAP_PROP_RTRIM:
		m->hash__ = _hash_rtrim;
		break;
	case MAP_PROP_NOCASE | MAP_PROP_RTRIM:
		m->hash__ = _hash_nocase_rtrim;
		break;
	default:
		m->hash__ = _hash;
	}

	memset(m->_entries.data, -1, sizeof(_Entry) * start_size);
	vec_construct_(&m->values, elem_size);
	vec_reserve_(&m->values, start_size / 2, elem_size);
}

void map_destroy(void* gen_m) {
	Map* m = gen_m;
	HEAP_FREE(m->_entries.data);
	HEAP_FREE(m->_keybuf.data);
	vec_destroy(&m->values);
}

void map_clear(void* gen_m) {
	Map* m = gen_m;
	vec_clear(&m->values);
	m->_keybuf_head = 0;
	memset(m->_entries.data, -1, sizeof(_Entry) * m->_entries.len);
}

uint32_t _map_declare(void* gen_m, const char* restrict key, unsigned n) {
	Map* m = gen_m;
	uint64_t hash = 0;
	_Entry*  e =
	    _get_entry(&m->_entries, &m->_keybuf, m->hash__, m->_keybuf_head, key, &n, &hash);

	if (e->val_idx != _NONE) {
		return e->val_idx;
	}

	/* new value at this point */
	e->key_idx = m->_keybuf_head;
	e->key_len = n;
	e->val_idx = m->values.size;
	e->hash    = hash;
	m->_keybuf_head += n;
	if (m->values.size > _FULL_PERCENT * m->_entries.len) {
		_map_grow_entries(&m->_entries);
	}
	return _NONE;
}

uint32_t map_nset_(void* gen_m, const char* restrict key, unsigned n, const void* data, int elem_size) {
	Map* m = gen_m;
	uint32_t idx = _map_declare(m, key, n);
	if (idx == _NONE) {
		vec_push_back_(&m->values, data, elem_size);
	} else {
		vec_set_one_at_(&m->values, idx, data, elem_size);
	}
	return idx;
}

void* map_nget_(void* gen_m, const char* restrict key, unsigned n, unsigned elem_size) {
	Map* m = gen_m;

	uint64_t hash = 0;
	_Entry*  e =
	    _get_entry(&m->_entries, &m->_keybuf, m->hash__, m->_keybuf_head, key, &n, &hash);

	if (e->val_idx == _NONE) { /* new value */
		return NULL;
	}

	return vec_iter_at_(&m->values, e->val_idx, elem_size);
}


void _map_grow_entries(_Entry_Slice* old_entries) {
	size_t old_start_size = old_entries->len;
	size_t new_start_size = _next_power_of_2(old_start_size + 1);

	_Entry_Slice new_entries = slice_new(_Entry, new_start_size);
	memset(new_entries.data, -1, sizeof(struct _Entry) * new_start_size);

	size_t i = 0;
	for (; i < old_start_size; ++i) {
		if (old_entries->data[i].val_idx == _NONE) {
			continue;
		}

		size_t         idx = (size_t)(old_entries->data[i].hash & (new_start_size - 1));
		struct _Entry* dest_entry = &new_entries.data[idx];

		while (dest_entry->val_idx != _NONE) {
			idx        = (idx + 1) % new_start_size;
			dest_entry = &new_entries.data[idx];
		}
		*dest_entry = old_entries->data[i];
	}

	HEAP_FREE(old_entries->data);
	*old_entries = new_entries;
}

uint64_t _hash(uint8_t* keybuf, const char* restrict key, unsigned* n) {
	uint64_t hash = _FNV1_INIT;
	unsigned i    = 0;

	for (; i < *n; ++i, ++keybuf) {
		*keybuf = key[i];
		hash *= _PRIME;
		hash ^= (uint64_t)*keybuf;
	}

	return hash;
}

uint64_t _hash_nocase(uint8_t* keybuf, const char* restrict key, unsigned* n) {
	uint64_t hash = _FNV1_INIT;
	unsigned i    = 0;

	for (; i < *n; ++i, ++keybuf) {
		*keybuf = tolower(key[i]);
		hash *= _PRIME;
		hash ^= (uint64_t)*keybuf;
	}

	return hash;
}

uint64_t _hash_rtrim(uint8_t* keybuf, const char* restrict key, unsigned* n) {
	unsigned last_not_space_n    = *n;
	uint64_t hash                = _FNV1_INIT;
	uint64_t last_not_space_hash = hash;
	unsigned i                   = 0;

	for (; i < *n; ++i, ++keybuf) {
		*keybuf = key[i];
		hash *= _PRIME;
		hash ^= (uint64_t)*keybuf;
		if (*keybuf != ' ') {
			last_not_space_hash = hash;
			last_not_space_n    = i + 1;
		}
	}

	*n = last_not_space_n;
	return last_not_space_hash;
}

uint64_t _hash_nocase_rtrim(uint8_t* keybuf, const char* restrict key, unsigned* n) {
	unsigned last_not_space_n    = *n;
	uint64_t hash                = _FNV1_INIT;
	uint64_t last_not_space_hash = hash;
	unsigned i                   = 0;

	for (; i < *n; ++i, ++keybuf) {
		*keybuf = tolower(key[i]);
		hash *= _PRIME;
		hash ^= (uint64_t)*keybuf;
		if (*keybuf != ' ') {
			last_not_space_hash = hash;
			last_not_space_n    = i + 1;
		}
	}

	*n = last_not_space_n;
	return last_not_space_hash;
}


_Entry* _get_entry(_Entry_Slice* entries,
    Byte_Slice*                  keybuf,
    hash_fn                      hash__,
    size_t                       keybuf_head,
    const char*                  key,
    unsigned*                    key_len,
    uint64_t*                    hash) {
	while (keybuf_head + *key_len > (size_t)keybuf->len) {
		keybuf->len *= 2;
		keybuf->data = heap_resize(keybuf->data, keybuf->len);
	}

	*hash      = hash__(&keybuf->data[keybuf_head], key, key_len);
	size_t idx = (size_t)(*hash & (entries->len - 1));

	/* use memcmp instead of strcmp in case non-char* key */
	_Entry* entry = &entries->data[idx];
	while (entry->val_idx != _NONE
	       && (entry->key_len != *key_len || entry->hash != *hash
	           || memcmp(&keybuf->data[entry->key_idx], &keybuf->data[keybuf_head], *key_len)
	                  != 0)) {
		idx   = (idx + 1) % entries->len;
		entry = &entries->data[idx];
	}

	return entry;
}

unsigned long _next_power_of_2(unsigned long n) {
	unsigned long value = 1;
	while (value < n) {
		value = value << 1;
	}
	return value;
}

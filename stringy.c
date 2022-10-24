#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "stringy.h"
#include <stdarg.h>
#include <string.h>
#include "util.h"

#define _terminate(s_) *(vec_end(*(s_))) = '\0'

void string_construct(String* s) {
	s = vec_construct(s);
	_terminate(s);
}

String string_make() {
	String s = {};
	vec_construct(&s);
	_terminate(&s);
	return s;
}

String string_from_string(String src) {
	String s = string_make();
	string_copy(&s, src);
	return s;
}

String string_from_slice(Const_Char_Slice s_slice) {
	String s = string_make();
	string_copy_from_slice(&s, s_slice);
	return s;
}

Char_Slice string_get_slice(String s) {
	return (Char_Slice) {
	    .data = s.data,
	    .len  = s.size,
	};
}

String string_from_char_ptr(const char* src) {
	String s = string_make();
	string_strcpy(&s, src);
	return s;
}

String string_take(char* src) {
	int    len = strlen(src) + 1;
	String s   = {
            .data   = src,
            .size   = len,
            ._alloc = len,
        };
	return s;
}

void string_insert(String* s, char* pos, const char* it, int n) {
	vec_insert(s, pos, it, n);
	_terminate(s);	
}
void string_insert_at(String* s, int idx, const char* it, int n) {
	vec_insert_at(s, idx, it, n);
	_terminate(s);
}
void string_insert_one(String* s, char* pos, char c) {
	vec_insert_one(s, pos, c);
	_terminate(s);
}
void string_insert_one_at(String* s, int idx, char c) {
	vec_insert_one_at(s, idx, c);
	_terminate(s);
}

void string_erase(String* s, char* pos, int n) {
	vec_erase(s, pos, n);
}
void string_erase_at(String* s, int idx, int n) {
	vec_erase_at(s, idx, n);
}
void string_erase_one(String* s, char* pos) {
	vec_erase_one(s, pos);
}
void string_erase_one_at(String* s, int idx) {
	vec_erase_one_at(s, idx);
}

void string_append(String* s, const char* it, int n) {
	vec_append(s, it, n);
	_terminate(s);
}
void string_extend(String* s, String src) {
	vec_extend(s, src);
}


void string_copy_from_slice(String* s, Const_Char_Slice cs) {
	string_strncpy(s, cs.data, cs.len);
}

void string_append_slice(String* dest, Const_Char_Slice cs) {
	int index = dest->size;
	vec_resize(dest, dest->size + cs.len);
	void* end = &vec_at(*dest, index);
	memcpy(end, cs.data, cs.len);
	_terminate(dest);
}

void string_push_back(String* s, char c) {
	char* back = vec_add_one(s);
	*back      = c;
	back[1]    = '\0';
}

int string_strcat(String* dest, const char* src) {
	int len    = strlen(src);
	int endidx = dest->size;
	vec_resize(dest, dest->size + len);
	void* end = &vec_at(*dest, endidx);
	memcpy(end, src, len + 1);
	return len;
}

int string_strncat(String* dest, const char* src, int n) {
	int len = strnlen(src, n);

	int old_size = dest->size;
	string_resize(dest, dest->size + len);
	char* end = &vec_at(*dest, old_size);
	memcpy(end, src, len);
	_terminate(dest);

	return dest->size - old_size;
}

int string_strcpy(String* dest, const char* src) {
	int len = strlen(src);
	string_resize(dest, len);
	memcpy(dest->data, src, len + 1);
	return len;
}

int string_strncpy(String* dest, const char* src, int limit) {
	string_resize(dest, limit);
	int i = 0;
	for (; src[i] != '\0' && i < limit; ++i) {
		((char*)dest->data)[i] = src[i];
	}
	string_resize(dest, i);
	_terminate(dest);
	return i - 1;
}

int string_sprintf(String* s, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	return string_vsprintf(s, fmt, args);
}

int string_vsprintf(String* s, const char* fmt, va_list args) {
	va_list args2;
	va_copy(args2, args);
	int len = vsnprintf(NULL, 0, fmt, args);
	va_end(args);
	vec_resize(s, len);
	vsnprintf(s->data, len + 1, fmt, args2);
	va_end(args2);
	return len;
}

const char* string_c_str(String s) {
	return (const char*)s.data;
}

char* string_export(String* s) {
	char* data = s->data;
	string_construct(s);
	return data;
}

void string_clear(String* s) {
	vec_clear(s);
	_terminate(s);
}

void string_copy(String* restrict dest, String src) {
	vec_resize(dest, src.size);
	memcpy(dest->data, src.data, src.size);
	_terminate(dest);
}

void string_resize(String* s, int n) {
	vec_resize(s, n);
	_terminate(s);
}

const char* string_find_replace_one(String* restrict s,
                                    const char* restrict oldstr,
                                    const char* restrict newstr,
                                    int begin_idx)
{
	unsigned newlen = strlen(newstr);
	return string_find_replace_one_limited(s, oldstr, newstr, begin_idx, newlen);
}

const char* string_find_replace_one_limited(String* restrict s,
                                            const char* restrict oldstr,
                                            const char* restrict newstr,
                                            int begin_idx,
                                            unsigned newlen)
{
	unsigned oldlen = strlen(oldstr);
	char* begin = &vec_at(*s, begin_idx);
	char* pos = memmem(begin, s->size - begin_idx, oldstr, oldlen);

	if (pos == NULL) {
		return vec_end(*s);
	}

	/* unnecessary optimization */
	if (newlen == oldlen) {
		memcpy(pos, newstr, newlen);
		return pos + newlen;
	}

	int pos_idx = vec_get_idx(*s, pos);

	vec_erase(s, pos, oldlen);
	vec_insert(s, pos, newstr, newlen);

	return vec_iter_at(*s, pos_idx + newlen);
}


void string_find_replace_limited(String* restrict s,
                                 const char* restrict oldstr,
                                 const char* restrict newstr,
                                 unsigned newlen)
{
	int i = 0;
	while (i < s->size) {
		const char* next =
		        string_find_replace_one_limited(s, oldstr, newstr, i, newlen);
		i = next - (const char*)vec_begin(*s);
	}
}


void string_find_replace(String* restrict s,
                         const char* restrict oldstr,
                         const char* restrict newstr)
{
	string_find_replace_limited(s, oldstr, newstr, strlen(newstr));
}

/* separate implementation for nocase limited because it must allocate */
const char* string_find_replace_one_nocase(String* restrict s,
                                           const char* restrict oldstr,
                                           const char* restrict newstr,
                                           int begin_idx)
{
	unsigned newlen = strlen(newstr);
	return string_find_replace_one_nocase_limited(s,
	                                              oldstr,
	                                              newstr,
	                                              begin_idx,
	                                              newlen);
}

const char* string_find_replace_one_nocase_limited(String* restrict s,
                                                   const char* restrict oldstr,
                                                   const char* restrict newstr,
                                                   int begin_idx,
                                                   unsigned newlen)
{
	unsigned oldlen = strlen(oldstr);
	char* begin = vec_iter_at(*s, begin_idx);

	/* We will assume oldstr is null terminated.
	 * We expect begin to be... It's a String...
	 */
	char* pos = strcasestr(begin, oldstr);

	if (pos == NULL) {
		return vec_end(*s);
	}

	int pos_idx = vec_get_idx(*s, pos);

	/* unnecessary optimization */
	if (newlen == oldlen) {
		memcpy(pos, newstr, newlen);
		return pos + newlen;
	}

	vec_erase(s, pos, oldlen);
	vec_insert(s, pos, newstr, newlen);

	return vec_iter_at(*s, pos_idx + newlen);
}


void string_find_replace_nocase_limited(String* restrict s,
                                        const char* restrict oldstr,
                                        const char* restrict newstr,
                                        unsigned newlen)
{
	int i = 0;
	while (i < s->size) {
		const char* next = string_find_replace_one_nocase_limited(s,
		                                                          oldstr,
		                                                          newstr,
		                                                          i,
		                                                          newlen);
		i = next - (const char*)vec_begin(*s);
	}
}


void string_find_replace_nocase(String* restrict s,
                                const char* restrict oldstr,
                                const char* restrict newstr)
{
	string_find_replace_nocase_limited(s, oldstr, newstr, strlen(newstr));
}

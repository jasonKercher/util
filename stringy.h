#ifndef STRINGY_H
#define STRINGY_H

#include "slice.h"
#include "vec.h"

/**
 * String is a vector of char with one exception:
 * in vec, there is always a trailing element
 * that is denoted as "end". that can contain
 * anything and should not be used. this is the
 * one difference here.  String will also have
 * this trailing element, but it will *always*
 * be a NULL terminator.
 */

typedef Vec(char) String;

/* create/destroy */
String string_make();
String string_from_string(const String);
String string_from_char_ptr(const char*);
String string_from_slice(Const_Char_Slice);
String string_take(char*);
#define string_destroy vec_destroy

/* access */
#define string_empty   vec_empty
#define string_at      vec_at

/* iterators */
#define string_begin   vec_begin
#define string_back    vec_back
#define string_end     vec_end
#define string_iter_at vec_iter_at

/* insertion */
void string_insert(String*, char* pos, const char* it, int n);
void string_insert_at(String*, int idx, const char* it, int n);
void string_insert_one(String*, char* pos, char);
void string_insert_one_at(String*, int idx, char);

/* deletion */
void string_erase(String*, char* pos, int n);
void string_erase_at(String*, int idx, int n);
void string_erase_one(String*, char* pos);
void string_erase_one_at(String*, int idx);

/* append */
void string_append(String*, const char* it, int n);
void string_extend(String*, String);

/* slice interface */
void string_copy_from_slice(String* s, Const_Char_Slice);
void string_append_slice(String*, Const_Char_Slice);
Char_Slice string_get_slice(String);

/* char interface */
void string_push_back(String*, char);
int string_strcat(String*, const char*);
int string_strncat(String*, const char*, int);
int string_strcpy(String*, const char*);
int string_strncpy(String*, const char*, int);
int string_sprintf(String* s, const char* fmt, ...);
int string_vsprintf(String* s, const char* fmt, va_list);
const char* string_c_str(String);

/* String iterface */
char* string_export(String*);
void string_clear(String*);
void string_resize(String*, int);
void string_copy(String* dest, String src);


/* Find and replace */
const char* string_find_replace_one(String* restrict,
                                    const char* restrict from,
                                    const char* restrict to,
                                    int);

void string_find_replace(String* restrict,
                         const char* restrict from,
                         const char* restrict to);

const char* string_find_replace_one_nocase(String* restrict,
                                           const char* restrict from,
                                           const char* restrict to,
                                           int);

void string_find_replace_nocase(String* restrict,
                                const char* restrict from,
                                const char* restrict to);

/* These are "one less allocation" options for possibly non-null terminated strings */
const char* string_find_replace_one_limited(String* restrict,
                                            const char* restrict from,
                                            const char* restrict to,
                                            int,
                                            unsigned);

void string_find_replace_limited(String* restrict,
                                 const char* restrict from,
                                 const char* restrict to,
                                 unsigned);

const char* string_find_replace_one_nocase_limited(String* restrict,
                                                   const char* restrict from,
                                                   const char* restrict to,
                                                   int,
                                                   unsigned);

void string_find_replace_nocase_limited(String*,
                                        const char* restrict from,
                                        const char* restrict to,
                                        unsigned);

#endif /* STRINGY_H */

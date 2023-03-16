#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>

#define PACKED __attribute__((__packed__))

#if   UINTPTR_MAX == 0xFF
#define POINTER_SIZE 1
#elif UINTPTR_MAX == 0xFFFF
#define POINTER_SIZE 2
#elif UINTPTR_MAX == 0xFFFFFFFF
#define POINTER_SIZE 4
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
#define POINTER_SIZE 8
#endif

#define STR(S_)   QUOTE(S_)
#define QUOTE(S_) #S_

#define UNREACHABLE()                                  \
	{                                              \
		fprintf(stderr,                        \
		    "\nHIT UNREACHABLE CODE ("__FILE__ \
		    ": %u\n",                          \
		    __LINE__);                         \
		abort();                               \
	}

#define hz_to_usec(HZ_)  ( 1000000 / (HZ_) )

#define NUM_COMPARE(a, b)             (((a) > (b)) - ((a) < (b)))
#define GET_MIN(A, B)                 ((A) < (B) ? (A) : (B))
#define GET_MAX(A, B)                 ((A) > (B) ? (A) : (B))
#define GET_MIN3(A, B, C)             MIN(MIN(A, B), C)
#define GET_MAX3(A, B, C)             MAX(MAX(A, B), C)
#define GET_CLAMP(X_, LOWER_, UPPER_) MIN(MAX((X_), (LOWER_)), (UPPER_))

/**
 * NOTE: Don't use if you understand caveats.
 */
#define ARRAY_LEN(ARR_) (sizeof(ARR_) / sizeof((ARR_)[0]))

enum Result {
	Result_Ok = 0,
	Result_Done = -5,
	Result_Fail = -1,
};
typedef enum Result Result;

typedef void (*generic_fn)(void);
typedef void (*generic_data_fn)(void*);
typedef int (*int_generic_data_fn)(void*);

/**
 * malloc wrapper that does error checking
 */
void* heap_alloc(long);

/**
 * realloc wrapper that does error checking
 */
void* heap_resize(void*, long);

/**
 * free pointer if not NULL and set to NULL
 * NOTE: breaking code standard setting pointer to NULL
 *       should be HEAP_FREE, but see no down side...
 */
#define heap_free(ptr_)                    \
	{                                  \
		if (ptr_ != NULL) {        \
			free((void*)ptr_); \
			ptr_ = NULL;       \
		}                          \
	}

/* Allow blank __VA_ARGS__ */
#define optarg_(...) , ##__VA_ARGS__

/* Allocate by type */
#define new(T_) heap_alloc(sizeof(T_));

#define NOT_IMPLEMENTED()                                                          \
	{                                                                          \
		fprintf(stderr, "not implemented (%s: %d)\n", __FILE__, __LINE__); \
		return Result_Fail;                                                \
	}

#define OR_RETURN(TEST_)                    \
	{                                   \
		Result res_ = (TEST_);      \
		if (res_ == Result_Fail) {  \
			return Result_Fail; \
		}                           \
	}

#define num_compare_(a, b) (((a) > (b)) - ((a) < (b)))

char* strncpy_safe(char* dest, const char* src, size_t n);


/**
 * Safely convert 2 bytes to uint16_t
 */
uint16_t bytes_to_u16(uint8_t b0_high, uint8_t b1_low);

/**
 * standard division, but round up instead
 * of truncating off any fraction
 */
int div_round_up(int dividend, int divisor);

/**
 * string to unsigned long, long and double conversion.
 * non-zero return is a failure. These functions print
 * error messages to stderr.
 */
int str2ulongbase(unsigned long* ret, const char* s, int base);
int str2ulong(unsigned long* ret, const char* s);
int str2longbase(long*, const char* s, int base);
int str2long(long*, const char* s);
int str2double(double*, const char* s);

/**
 * charcount simply counts the occurences of char c
 * in the string s.
 */
int charcount(const char* s, char c);
/**
 * Same as charcount except for non-null terminated
 * strings.
 */
int charncount(const char* s, char c, unsigned n);

/**
 * strhaschar checks whether a char c exists within string s.
 *
 * returns:
 *      - 1 if yes
 *      - 0 if no
 */
int strhaschar(const char* s, char c);

/**
 * removecharat shifts all characters left 1 at the
 * provided index i in order to essentially remove that
 * character from the string.
 */
void removecharat(char* s, int i);

/**
 * randstr generates a random string of length n
 * out of all alpha-numeric characters.
 *
 * returns:
 *      - char* of random characters
 */
char* randstr(char* s, const int n);

/**
 * getnoext assigns a filename with no extension
 */
void getnoext(char* dest, const char* filename);

/**
 * lower case a char* in place
 * NOTE: NULL terminator is assumed.
 */
void string_to_lower(char* s);

#ifdef __cplusplus
}
#endif

#endif

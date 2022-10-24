#include "util.h"

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

/////////////////////////////////////////////////
/// Helper function to check for malloc errors
void* heap_alloc(long size) {
	void* allocation = malloc(size);
	if (allocation == NULL) {
		perror("malloc");
		abort();
	}
	return allocation;
}

/////////////////////////////////////////////////
/// Helper function to check for realloc errors
void* heap_resize(void* data, long size) {
	void* new_data = realloc(data, size);
	if (!new_data) {
		perror("realloc");
		abort();
	}
	return new_data;
}


/////////////////////////////////////////////////
/// safe strncpy that will always be 0 terminated
char* strncpy_safe(char* dest, const char* src, size_t n) {
	char* res   = strncpy(dest, src, n - 1);
	dest[n - 1] = '\0';
	return res;
}

/////////////////////////////////////////////////
/// safely read u16 from byte buffer
/// The following will crash/reset:
///
/// uint8_t a[5];
/// uint16_t* ptr = (uint16_t*)&a[1];
///
inline uint16_t bytes_to_u16(uint8_t b0, uint8_t b1) {
	uint16_t val = b1;
	val <<= 8;
	return val | b0;
}


/////////////////////////////////////////////////
/// Divide by divisor and round up to nearest
/// muliple of the divisor
int div_round_up(int dividend, int divisor) {
	return dividend / divisor + dividend % divisor;
}

int str2ulongbase(unsigned long* ret, const char* s, int base)
{
	char* endptr = NULL;
	errno = 0;
	*ret = strtoul(s, &endptr, base);
	if (errno) {
		perror(s);
	} else if (errno == 0 && s && *endptr != 0) {
		fprintf(stderr, "%s: could not convert to unsigned integer\n", s);
	} else {
		return 0;
	}

	return 1;
}

int str2ulong(unsigned long* ret, const char* s)
{
	if (s[0] == '0' && s[1] == 'x' && isdigit(s[2])) {
		return str2ulongbase(ret, &s[2], 16);
	}
	if (s[0] == '0' && isdigit(s[1])) {
		return str2ulongbase(ret, &s[1], 8);
	}
	if (s[0] == '0' && s[1] == 'b' && isdigit(s[2])) {
		return str2ulongbase(ret, &s[2], 2);
	}
	return str2ulongbase(ret, s, 10);
}

int str2longbase(long* ret, const char* s, int base)
{
	char* endptr = NULL;
	errno = 0;
	*ret = strtol(s, &endptr, base);
	if (errno) {
		perror(s);
	} else if (errno == 0 && s && *endptr != 0) {
		fprintf(stderr, "%s: could not convert to integer\n", s);
	} else {
		return 0;
	}

	return 1;
}

int str2long(long* ret, const char* s)
{
	if (s[0] == '0' && s[1] == 'x' && isdigit(s[2])) {
		return str2longbase(ret, &s[2], 16);
	}
	if (s[0] == '0' && isdigit(s[1])) {
		return str2longbase(ret, &s[1], 8);
	}
	if (s[0] == '0' && s[1] == 'b' && isdigit(s[2])) {
		return str2longbase(ret, &s[2], 2);
	}
	return str2longbase(ret, s, 10);
}

int str2double(double* ret, const char* s)
{
	char* endptr = NULL;
	errno = 0;
	*ret = strtod(s, &endptr);

	if (errno) {
		perror(s);
	} else if (errno == 0 && s && *endptr != 0) {
		fprintf(stderr, "%s: could not convert to float\n", s);
	} else {
		return 0;
	}

	return 1;
}

int charncount(const char* s, char c, unsigned n)
{
	int count = 0;
	unsigned i = 0;
	for(; s[i] != '\0' && i < n; ++i)
		if (s[i] == c)
			++count;

	return count;
}

int charcount(const char* s, char c)
{
	int count = 0;
	int i = 0;
	for(; s[i] != '\0'; ++i)
		if (s[i] == c)
			++count;

	return count;
}

int strhaschar(const char* s, char c)
{
	int i = 0;
	for (; s[i] != '\0'; ++i)
		if (s[i] == c)
			return true;

	return false;
}

void removecharat(char* s, int i)
{
	for (; s[i] != '\0'; ++i)
		s[i] = s[i+1];
	s[i] = '\0';
}

char* randstr(char* s, const int len)
{
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	int i = 0;

	for (; i < len; ++i)
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];

	s[i] = '\0';

	return s;
}

/* https://stackoverflow.com/questions/2736753/how-to-remove-extension-from-file-name
 * converted to leave user in control of allocation
 * assumption: dest space >= filename space
 */
void getnoext(char* dest, const char* filename)
{
	char* lastdot = NULL;
	char* lastsep = NULL;

	if (!filename) {
		dest[0] = '\0';
		return;
	}

	strcpy(dest, filename);
	lastdot = strrchr(dest, '.');
	lastsep = ('/' == 0) ? NULL : strrchr(dest, '/');

	if (lastdot != NULL) {
		if (lastsep != NULL) {
			if (lastsep < lastdot)
				*lastdot = '\0';
		} else {
			*lastdot = '\0';
		}
	}
}

void string_to_lower(char* s)
{
	for ( ; *s; ++s)
		*s = tolower(*s);
}

char* strnstr(const char* s, const char* find, size_t slen)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if (slen-- < 1 || (sc = *s++) == '\0')
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

#if 0
Node* dir_list_files(const char* dir)
{
	/* read file names into node */
	Node* files = NULL;
	DIR *dr;
	struct dirent *en;

	dr = opendir(dir); //open all or present directory
	if (!dr) {
		perror(dir);
		exit(EXIT_FAILURE);
	}

	while ((en = readdir(dr)) != NULL) {
		node_enqueue(&files, strdup(en->d_name));
	}
	closedir(dr);

	return files;
}
#endif

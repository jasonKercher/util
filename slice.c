#include "slice.h"
#include <string.h>
#include "util.h"

Slice slice_new_(unsigned size, size_t len)
{
	Slice s = {
		.data = heap_alloc(size * len),
		.len = len,
	};
	return s;
}

int slice_compare_nocase(const Slice* restrict sv0,
                              const Slice* restrict sv1)
{
	int len = (sv0->len > sv1->len) ? sv1->len : sv0->len;
	int ret = num_compare_(sv0->len, sv1->len);
	int maybe_ret = strncasecmp(sv0->data, sv1->data, len);
	if (maybe_ret) {
		return maybe_ret;
	}
	return ret;
}

int slice_compare(const Slice* restrict sv0, const Slice* restrict sv1)
{
	int len = (sv0->len > sv1->len) ? sv1->len : sv0->len;
	int ret = num_compare_(sv0->len, sv1->len);
	int maybe_ret = strncmp(sv0->data, sv1->data, len);
	if (maybe_ret) {
		return maybe_ret;
	}
	return ret;
}

int slice_compare_rtrim(const Slice* sv0, const Slice* sv1)
{
	const Slice* short_sv = sv0;
	const Slice* long_sv = sv1;
	if (sv0->len > sv1->len) {
		short_sv = sv1;
		long_sv = sv0;
	}

	const unsigned char *s0 = (unsigned char*) sv0->data;
	const unsigned char *s1 = (unsigned char*) sv1->data;

	int ret = 0;
	unsigned i = 0;
	for (; ret == 0 && i < short_sv->len; ++i) {
		ret = s0[i] - s1[i];
	}

	/* All remaining bytes must be white space to stay truthy */
	const unsigned char *long_str = (unsigned char*) long_sv->data;

	for (; ret == 0 && i < long_sv->len; ++i) {
		ret = (isspace(long_str[i])) ? 0 : sv0->len - sv1->len;
	}
	return ret;
}

int slice_compare_nocase_rtrim(const Slice* sv0, const Slice* sv1)
{
	const Slice* short_sv = sv0;
	const Slice* long_sv = sv1;
	if (sv0->len > sv1->len) {
		short_sv = sv1;
		long_sv = sv0;
	}

	const unsigned char* s0 = (unsigned char*)sv0->data;
	const unsigned char* s1 = (unsigned char*)sv1->data;

	int ret = 0;
	unsigned i = 0;
	for (; ret == 0 && i < short_sv->len; ++i) {
		ret = tolower (s0[i]) - tolower (s1[i]);
	}

	/* All remaining bytes must be white space to stay truthy */
	const unsigned char *long_str = (unsigned char*) long_sv->data;

	for (; ret == 0 && i < long_sv->len; ++i) {
		ret = (isspace(long_str[i])) ? 0 : sv0->len - sv1->len;
	}
	return ret;
}


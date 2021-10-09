#include <stdio.h>
#include <string.h>
#include "util.h"
#include "hashmap.h"

int main(int argc, char** argv)
{
	hashmap* m = new_t_(hashmap, int, 16, HASHMAP_PROP_NOCASE);

	int one = 1;

	hashmap_set(m, "one", &one);
	int* iptr = hashmap_get(m, "ONE");
	if (iptr != NULL) {
		printf("%d\n", *iptr);
	}

	return 0;
}

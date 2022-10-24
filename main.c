#include <assert.h>
#include <stdio.h>
#include "vec.h"
#include "util.h"
#include "hashmap.h"

int one = 1;
int two = 2;
int three = 3;
int ten = 10;
int twelve = 12;
int test = 100;
int Test = 101;
int test_ = 102;
int varible_size_key = 141;

typedef Hashmap(int) Int_Map;

void sets(Int_Map* m)
{
	hashmap_set(m, "one", ten);
	hashmap_set(m, "two", two);
	hashmap_set(m, "one", one);
	hashmap_set(m, "ten", ten);
	hashmap_set(m, "twelve", twelve);
	hashmap_set(m, "test", test);
	hashmap_set(m, "Test", Test);
	hashmap_set(m, "test ", test_);
}

void test_hashmap_basic()
{
	Int_Map m;
	hashmap_construct(&m, int, 20, HASHMAP_PROP_DEFAULT);

	sets(&m);

	int* val = 0;
	val = hashmap_get(&m, "one");
	assert(*val == one);
	val = hashmap_get(&m, "two");
	assert(*val == two);
	val = hashmap_get(&m, "ten");
	assert(*val == ten);
	val = hashmap_get(&m, "twelve");
	assert(*val == twelve);
	val = hashmap_get(&m, "test");
	assert(*val == test);
	val = hashmap_get(&m, "Test");
	assert(*val == Test);
	val = hashmap_get(&m, "test ");
	assert(*val == test_);
	void* no_match = hashmap_get(&m, "no");
	assert(no_match == NULL);

	hashmap_destroy(&m);
}

void test_hashmap_nocase()
{
	Int_Map m;
	hashmap_construct(&m, int, 20, HASHMAP_PROP_NOCASE);

	sets(&m);

	int* val = 0;
	val = hashmap_get(&m, "one");
	assert(*val == one);
	val = hashmap_get(&m, "two");
	assert(*val == two);
	val = hashmap_get(&m, "ten");
	assert(*val == ten);
	val = hashmap_get(&m, "twelve");
	assert(*val == twelve);
	val = hashmap_get(&m, "test");
	assert(*val == Test);
	val = hashmap_get(&m, "Test");
	assert(*val == Test);
	val = hashmap_get(&m, "test ");
	assert(*val == test_);
	void* no_match = hashmap_get(&m, "no");
	assert(no_match == NULL);

	hashmap_destroy(&m);
}

void test_hashmap_rtrim()
{
	Int_Map m;
	hashmap_construct(&m, int, 20, HASHMAP_PROP_RTRIM);

	sets(&m);
	
	int* val = 0;
	val = hashmap_get(&m, "one");
	assert(*val == one);
	val = hashmap_get(&m, "two");
	assert(*val == two);
	val = hashmap_get(&m, "ten");
	assert(*val == ten);
	val = hashmap_get(&m, "twelve");
	assert(*val == twelve);
	val = hashmap_get(&m, "test");
	assert(*val == test_);
	val = hashmap_get(&m, "Test");
	assert(*val == Test);
	val = hashmap_get(&m, "test ");
	assert(*val == test_);
	void* no_match = hashmap_get(&m, "no");
	assert(no_match == NULL);

	hashmap_destroy(&m);
}

void test_hashmap_nocase_rtrim()
{
	Int_Map m;
	hashmap_construct(&m, int, 20, HASHMAP_PROP_RTRIM | HASHMAP_PROP_NOCASE);

	sets(&m);
	
	int* val = 0;
	val = hashmap_get(&m, "one");
	assert(*val == one);
	val = hashmap_get(&m, "two");
	assert(*val == two);
	val = hashmap_get(&m, "ten");
	assert(*val == ten);
	val = hashmap_get(&m, "twelve");
	assert(*val == twelve);
	val = hashmap_get(&m, "test");
	assert(*val == test_);
	val = hashmap_get(&m, "Test");
	assert(*val == test_);
	val = hashmap_get(&m, "test ");
	assert(*val == test_);
	void* no_match = hashmap_get(&m, "no");
	assert(no_match == NULL);

	hashmap_destroy(&m);
}

int main(void)
{
	test_hashmap_basic();
	test_hashmap_nocase();
	test_hashmap_rtrim();
	test_hashmap_nocase_rtrim();
}

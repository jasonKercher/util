#include <assert.h>
#include <stdio.h>
#include "vec.h"
#include "util.h"
#include "map.h"

int one = 1;
int two = 2;
int three = 3;
int ten = 10;
int twelve = 12;
int test = 100;
int Test = 101;
int test_ = 102;
int varible_size_key = 141;

typedef Map(int) Int_Map;

void sets(Int_Map* m)
{
	map_set(m, "one", ten);
	map_set(m, "two", two);
	map_set(m, "one", one);
	map_set(m, "ten", ten);
	map_set(m, "twelve", twelve);
	map_set(m, "test", test);
	map_set(m, "Test", Test);
	map_set(m, "test ", test_);
}

void test_map_basic()
{
	Int_Map m;
	map_construct(&m, 20, MAP_PROP_DEFAULT);

	sets(&m);

	int* val = 0;
	val = map_get(&m, "one");
	assert(*val == one);
	val = map_get(&m, "two");
	assert(*val == two);
	val = map_get(&m, "ten");
	assert(*val == ten);
	val = map_get(&m, "twelve");
	assert(*val == twelve);
	val = map_get(&m, "test");
	assert(*val == test);
	val = map_get(&m, "Test");
	assert(*val == Test);
	val = map_get(&m, "test ");
	assert(*val == test_);
	void* no_match = map_get(&m, "no");
	assert(no_match == NULL);

	map_destroy(&m);
}

void test_map_nocase()
{
	Int_Map m;
	map_construct(&m, 20, MAP_PROP_NOCASE);

	sets(&m);

	int* val = 0;
	val = map_get(&m, "one");
	assert(*val == one);
	val = map_get(&m, "two");
	assert(*val == two);
	val = map_get(&m, "ten");
	assert(*val == ten);
	val = map_get(&m, "twelve");
	assert(*val == twelve);
	val = map_get(&m, "test");
	assert(*val == Test);
	val = map_get(&m, "Test");
	assert(*val == Test);
	val = map_get(&m, "test ");
	assert(*val == test_);
	void* no_match = map_get(&m, "no");
	assert(no_match == NULL);

	map_destroy(&m);
}

void test_map_rtrim()
{
	Int_Map m;
	map_construct(&m, 20, MAP_PROP_RTRIM);

	sets(&m);
	
	int* val = 0;
	val = map_get(&m, "one");
	assert(*val == one);
	val = map_get(&m, "two");
	assert(*val == two);
	val = map_get(&m, "ten");
	assert(*val == ten);
	val = map_get(&m, "twelve");
	assert(*val == twelve);
	val = map_get(&m, "test");
	assert(*val == test_);
	val = map_get(&m, "Test");
	assert(*val == Test);
	val = map_get(&m, "test ");
	assert(*val == test_);
	void* no_match = map_get(&m, "no");
	assert(no_match == NULL);

	map_destroy(&m);
}

void test_map_nocase_rtrim()
{
	Int_Map m;
	map_construct(&m, 20, MAP_PROP_RTRIM | MAP_PROP_NOCASE);

	sets(&m);
	
	int* val = 0;
	val = map_get(&m, "one");
	assert(*val == one);
	val = map_get(&m, "two");
	assert(*val == two);
	val = map_get(&m, "ten");
	assert(*val == ten);
	val = map_get(&m, "twelve");
	assert(*val == twelve);
	val = map_get(&m, "test");
	assert(*val == test_);
	val = map_get(&m, "Test");
	assert(*val == test_);
	val = map_get(&m, "test ");
	assert(*val == test_);
	void* no_match = map_get(&m, "no");
	assert(no_match == NULL);

	map_destroy(&m);
}

int main(void)
{
	test_map_basic();
	test_map_nocase();
	test_map_rtrim();
	test_map_nocase_rtrim();
}

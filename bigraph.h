#ifndef BIGRAPH_H
#define BIGRAPH_H

#include <stdbool.h>

#include "vec.h"
#include "fifo.h"

struct Binode {
	void* data;
	struct Binode* out[2];
	int visit_count;
	bool is_root;
};
typedef struct Binode Binode;

struct Binode* Binode_construct(struct Binode* restrict, void* restrict);

typedef Vec(struct Binode*) Binode_Vec;
typedef Fifo(struct Binode*) Binode_Fifo;

struct Bigraph {
	Binode_Vec nodes;
	struct Binode* newest;
	Binode_Fifo _trav;
	Binode_Vec _roots;
	int _root_idx;
	bool _roots_good;
};
typedef struct Bigraph Bigraph;

struct Bigraph* bigraph_construct(struct Bigraph* restrict);
void bigraph_shallow_free(struct Bigraph* restrict);
void bigraph_shallow_destroy(struct Bigraph* restrict);
void bigraph_destroy(struct Bigraph* restrict);

struct Binode* bigraph_add_node(struct Bigraph* restrict, struct Binode*);
struct Binode* bigraph_add_data(struct Bigraph* restrict, void* restrict);
void bigraph_consume(struct Bigraph* restrict dest, struct Bigraph* restrict src);

void* bigraph_remove(struct Bigraph* restrict, struct Binode**);

unsigned bigraph_root_count(struct Bigraph* restrict);
Binode_Vec* bigraph_get_roots(struct Bigraph* restrict);
void bigraph_traverse_reset(struct Bigraph* restrict);
struct Binode* bigraph_traverse(struct Bigraph* restrict);

#endif /* BIGRAPH_H */

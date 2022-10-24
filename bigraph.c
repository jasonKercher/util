#include "bigraph.h"

#include "util.h"

Binode* Binode_construct(Binode* restrict node, void* restrict data)
{
	*node = (Binode) {
	        .data = data,
	};

	return node;
}

Bigraph* bigraph_construct(Bigraph* restrict self)
{
	*self = (Bigraph) {
	        ._root_idx = 0,
	};

	vec_construct(&self->nodes);
	vec_construct(&self->_roots);
	fifo_construct(&self->_trav, 5);

	return self;
}

void bigraph_shallow_free(Bigraph* restrict self)
{
	bigraph_shallow_destroy(self);
	heap_free(self);
}

void bigraph_shallow_destroy(Bigraph* restrict self)
{
	vec_destroy(&self->_roots);
	vec_destroy(&self->nodes);
	fifo_destroy(&self->_trav);
}

void bigraph_destroy(Bigraph* restrict self)
{
	int i = 0;
	Binode** node = vec_begin(self->nodes);
	for (; i < self->nodes.size; ++i) {
		heap_free(node[i]);
	}
	bigraph_shallow_destroy(self);
}

/* making a copy here */
Binode* bigraph_add_node(Bigraph* restrict self, Binode* node)
{
	vec_push_back(&self->nodes, node);
	self->newest = node;
	self->_roots_good = false;
	return node;
}

Binode* bigraph_add_data(Bigraph* restrict self, void* restrict data)
{
	Binode* node = new(Binode);
	*node = (Binode) {
		.data = data,
	};
	return bigraph_add_node(self, node);
}

void bigraph_consume(Bigraph* restrict dest, Bigraph* restrict src)
{
	dest->_roots_good = false;
	vec_extend(&dest->nodes, src->nodes);
	bigraph_shallow_free(src);
}

void* bigraph_remove(Bigraph* restrict self, Binode** node)
{
	Binode** it = vec_begin(self->nodes);
	for (; it != vec_end(self->nodes); ++it) {
		if ((*it)->out[0] == *node) {
			(*it)->out[0] = NULL;
		}
		if ((*it)->out[1] == *node) {
			(*it)->out[1] = NULL;
		}
	}
	void* data = (*node)->data;
	heap_free(node);
	vec_erase_one(&self->nodes, node);
	self->_roots_good = false;
	return data;
}

int _assume_roots(Bigraph* restrict self)
{
	if (vec_empty(self->nodes)) {
		return 0;
	}

	/* reset */
	Binode** it = vec_begin(self->nodes);
	for (; it != vec_end(self->nodes); ++it) {
		(*it)->visit_count = 0;
	}

	/* get counts */
	it = vec_begin(self->nodes);
	for (; it != vec_end(self->nodes); ++it) {
		if ((*it)->out[0] != NULL) {
			++(*it)->out[0]->visit_count;
		}
		if ((*it)->out[1] != NULL) {
			++(*it)->out[1]->visit_count;
		}
	}

	it = vec_begin(self->nodes);
	for (; it != vec_end(self->nodes); ++it) {
		if ((*it)->visit_count == 0) {
			(*it)->is_root = true;
			vec_push_back(&self->_roots, *it);
			continue;
		}
		(*it)->visit_count = 0;
	}

	return self->_roots.size;
}

unsigned bigraph_root_count(Bigraph* restrict self)
{
	return self->_roots.size;
}

Binode_Vec* bigraph_get_roots(Bigraph* restrict self)
{
	if (self->_roots_good) {
		return &self->_roots;
	}

	vec_clear(&self->_roots);
	Binode** it = vec_begin(self->nodes);
	for (; it != vec_end(self->nodes); ++it) {
		if ((*it)->is_root) {
			vec_push_back(&self->_roots, *it);
		}
	}

	/* if no roots found, try to figure it out */
	if (vec_empty(self->_roots)) {
		/* TODO: handle return of 0 */
		_assume_roots(self);
	}
	self->_roots_good = true;
	return &self->_roots;
}

void bigraph_traverse_reset(Bigraph* restrict self)
{
	bigraph_get_roots(self);

	fifo_resize(&self->_trav, self->nodes.size);

	/* we are going to enter the first root manually */
	self->_root_idx = 1;

	Binode** it = vec_begin(self->nodes);
	for (; it != vec_end(self->nodes); ++it) {
		(*it)->visit_count = 0;
	}

	/* start at first discovered root */
	fifo_add(&self->_trav, vec_at(self->_roots, 0));
}

Binode* bigraph_traverse(Bigraph* restrict self)
{
	while (fifo_is_empty(&self->_trav)) {
		if (self->_root_idx >= self->_roots.size) {
			return NULL;
		}
		Binode* node = vec_at(self->_roots, self->_root_idx++);
		if (node->visit_count) {
			continue;
		}
		fifo_add(&self->_trav, node);
		return bigraph_traverse(self);
	}

	Binode** node = fifo_get(&self->_trav);
	++(*node)->visit_count;

	if ((*node)->out[0] != NULL && !(*node)->out[0]->visit_count) {
		fifo_add(&self->_trav, (*node)->out[0]);
	}
	if ((*node)->out[1] != NULL && !(*node)->out[1]->visit_count) {
		fifo_add(&self->_trav, (*node)->out[1]);
	}

	return *node;
}

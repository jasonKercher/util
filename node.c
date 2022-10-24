#include "util.h"
#include "node.h"

void* _node_remove(Node* Node);

/* STACK FUNCTIONS */
Node* node_top(Node* restrict self)
{
	if (!self)
		return NULL;

	while (self && self->prev)
		self = self->prev;

	return self;
}

Node* node_bottom(Node* restrict self)
{
	if (!self)
		return NULL;

	while (self && self->next)
		self = self->next;

	return self;
}

Node* _node_pop(Node** head)
{
	if (*head == NULL) {
		return NULL;
	}
	Node* oldhead = *head;
	*head = oldhead->next;
	return oldhead;
}

Node* node_pop_export(Node** head)
{
	Node* oldhead = _node_pop(head);
	return node_export(oldhead);
}

void* node_pop(Node** head)
{
	Node* oldhead = _node_pop(head);
	return _node_remove(oldhead);
}

Node* node_push(Node** head, void* restrict data)
{
	Node* newnode = heap_alloc(sizeof(*newnode));
	*newnode = (Node) {
	        .data = data,
	        .next = *head,
	};

	node_push_import(head, newnode);
	return newnode;
}

Node* node_push_import(Node** head, Node* restrict import)
{
	import->next = *head;

	if (*head)
		(*head)->prev = import;

	import->prev = NULL;

	*head = import;
	return import;
}

/* QUEUE FUNCTIONS */
Node* node_front(Node* restrict self)
{
	if (!self)
		return NULL;

	while (self && self->prev)
		self = self->prev;

	return self;
}

Node* node_back(Node* restrict self)
{
	if (!self)
		return NULL;

	while (self && self->next)
		self = self->next;

	return self;
}

Node* node_enqueue_import(Node** head, Node* restrict import)
{
	Node* back = node_back(*head);

	if (back) {
		import->prev = back;
		back->next = import;
	}

	if (!*head)
		*head = import;

	import->next = NULL;

	return import;
}

Node* node_enqueue(Node** head, void* restrict data)
{
	Node* newnode = heap_alloc(sizeof(*newnode));
	*newnode = (Node) {
	        .data = data,
	};

	node_enqueue_import(head, newnode);
	return newnode;
}

/** generic linked list functions **/
Node* node_at(Node* restrict head, unsigned idx)
{
	unsigned i = 0;
	for (; head && i < idx; ++i) {
		head = head->next;
	}

	return head;
}

void* node_data_at(Node* restrict head, unsigned idx)
{
	Node* Node = node_at(head, idx);
	if (Node == NULL) {
		return NULL;
	}
	return Node->data;
}

int node_count(Node* restrict head)
{
	if (!head)
		return 0;

	int count = 1;
	while ((head = head->next))
		++count;

	return count;
}

Node* node_export(Node* restrict export)
{
	if (!export)
		return NULL;

	if (export->next)
		export->next->prev = export->prev;
	if (export->prev)
		export->prev->next = export->next;

	export->next = NULL;
	export->prev = NULL;

	return export;
}

void* _node_remove(Node* Node)
{
	if (Node == NULL) {
		return NULL;
	}
	void* data = Node->data;
	node_export(Node);
	heap_free(Node);
	return data;
}

void* node_remove(Node** head, Node* Node)
{
	if (!Node)
		return NULL;

	if (*head == Node) {
		return node_pop(head);
	}
	return _node_remove(Node);
}

void node_delete(Node** head, Node* Node)
{
	void* data = node_remove(head, Node);
	heap_free(data);
}

void node_free_func(Node** head, generic_data_fn free_func)
{
	*head = node_top(*head);
	for (; *head; node_pop(head))
		free_func((*head)->data);
}

void node_free_data(Node** head)
{
	*head = node_top(*head);
	while (*head) {
		void* data = node_pop(head);
		heap_free(data);
	}
}

void node_free(Node** head)
{
	*head = node_top(*head);
	for (; *head; node_pop(head))
		;
}

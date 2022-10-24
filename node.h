#ifndef NODE_H
#define NODE_H

#ifdef __cplusplus
extern "C" {
#define restrict
#endif

struct Node {
	void* data;
	struct Node* prev;
	struct Node* next;
};
typedef struct Node Node;

/** NOTE: mutators need reference to Node* **/

/* Treat nodes as a queue */
struct Node* node_front(struct Node* restrict Node);
struct Node* node_back(struct Node* restrict Node);
struct Node* node_enqueue(struct Node** head, void* retrict);
struct Node* node_enqueue_import(struct Node** head, struct Node* restrict);
#define node_dequeue(head_)        node_pop(head_)
#define node_dequeue_export(head_) node_pop_export(head_)

/* Treat nodes as a stack */
struct Node* node_top(struct Node* restrict Node);
struct Node* node_bottom(struct Node* restrict Node);
struct Node* node_push(struct Node** head, void* restrict);
struct Node* node_push_import(struct Node** head, struct Node* restrict);
void* node_pop(struct Node** head);
struct Node* node_pop_export(struct Node** head);

void* node_data_at(struct Node* restrict, unsigned);
struct Node* node_at(struct Node* restrict, unsigned);
int node_count(struct Node* restrict head);

struct Node* node_export(struct Node* restrict);
void* node_remove(struct Node** head, struct Node* restrict Node);
void node_delete(struct Node** head, struct Node* restrict Node);
void node_free_func(struct Node**, void (*)(void*));
void node_free_data(struct Node** head);
void node_free(struct Node** head);

#ifdef __cplusplus
}
#endif

#endif  /* NODE_H */

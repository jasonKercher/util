#ifndef BITVEC_H
#define BITVEC_H

#include "vec.h"
#include <stdint.h>

typedef Vec(uint32_t) Bitvec;

Bitvec* bitvec_construct(Bitvec*);
void bitvec_destroy(Bitvec*);
bool bitvec_at(Bitvec, int idx);
void bitvec_resize(Bitvec*, int);
void bitvec_set(Bitvec*, int idx);
void bitvec_unset(Bitvec*, int idx);
void bitvec_push_back(Bitvec*, bool);

#endif /* BITVEC_H */

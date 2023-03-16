#include "types.h"


u8
type_iter_u8(Type_Iterator* t) {
	return *(*t)++;
}
u16
type_iter_u16(Type_Iterator* t) {
	uint16_t val = *(*t)++;
	val |= ((uint16_t) * (*t)++ << 8);
	return val;
}
u32
type_iter_u32(Type_Iterator* t) {
	uint32_t val = *(*t)++;
	val |= ((uint32_t) * (*t)++ << 8);
	val |= ((uint32_t) * (*t)++ << 16);
	val |= ((uint32_t) * (*t)++ << 24);
	return val;
}
u64
type_iter_u64(Type_Iterator* t) {
	uint64_t val = *(*t)++;
	val |= ((uint64_t) * (*t)++ << 8);
	val |= ((uint64_t) * (*t)++ << 16);
	val |= ((uint64_t) * (*t)++ << 24);
	val |= ((uint64_t) * (*t)++ << 32);
	val |= ((uint64_t) * (*t)++ << 40);
	val |= ((uint64_t) * (*t)++ << 48);
	val |= ((uint64_t) * (*t)++ << 56);
	return val;
}

i8
type_iter_i8(Type_Iterator* t) {
	return (i8)type_iter_u8(t);
}
i16
type_iter_i16(Type_Iterator* t) {
	return (i16)type_iter_u16(t);
}
i32
type_iter_i32(Type_Iterator* t) {
	return (i32)type_iter_u32(t);
}
i64
type_iter_i64(Type_Iterator* t) {
	return (i64)type_iter_u64(t);
}

#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uintptr_t uintptr;
typedef intptr_t  intptr;
typedef unsigned  uint;

typedef uint8_t* Type_Iterator;

u8  type_iter_u8(Type_Iterator*);
u16 type_iter_u16(Type_Iterator*);
u32 type_iter_u32(Type_Iterator*);
u64 type_iter_u64(Type_Iterator*);

i8  type_iter_i8(Type_Iterator*);
i16 type_iter_i16(Type_Iterator*);
i32 type_iter_i32(Type_Iterator*);
i64 type_iter_i64(Type_Iterator*);

#endif /* TYPE_H */

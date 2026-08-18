#pragma once 

#include <stdlib.h>
#include <memory.h>
#include <assert.h>

enum{
    VEC_INVALID_PARAM = 1,
    VEC_RESIZE_FAILED = 2,
    VEC_TYPE_MISMATCH = 3
};

typedef struct Vector Vector;

Vector* vector_construct(size_t data_type_size);

int vector_destroy(Vector* vector);

static Vector* vector_resize(Vector* vector, size_t new_capacity);

int vector_append(Vector** vector, void* data);

int vector_concat(Vector** dest_vector, const Vector*  src_vector);

void* vector_get(Vector* vector, size_t index);

int vector_remove(Vector** vector, size_t index);

size_t vector_get_size(Vector* vector);

size_t vector_get_data_type_size(Vector* vector);
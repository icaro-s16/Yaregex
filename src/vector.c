#include "vector.h"


// Private declarations

const int VEC_BASE_SIZE = 16;

struct Vector{
    size_t capacity;
    size_t size;
    size_t data_type_size;
    void* vector;
};

Vector* vector_construct(size_t data_type_size){
    if (data_type_size <= 0)
        return NULL;
    Vector* vec = malloc(sizeof(Vector));
    if (!vec)
        return NULL;
    vec->data_type_size = data_type_size;
    vec->capacity = VEC_BASE_SIZE;
    vec->size = 0;
    if(!(vec->vector = calloc(data_type_size, vec->capacity)))
        return NULL;
    return vec;
}

int vector_destroy(Vector* vector){
    if (vector == NULL)
        return -1;
    if (vector->vector != NULL ) free(vector->vector);
    free(vector);
    return 0;
}

static Vector* vector_resize(Vector* vector, size_t new_capacity){
    assert(vector != NULL);
    assert(new_capacity > 0);
    
    vector->capacity = new_capacity;
    void *new_vec = (void*)reallocarray(vector->vector, vector->capacity, vector->data_type_size);
    if (new_vec == NULL) 
        return NULL;

    vector->vector = new_vec;
    return vector;
}

int vector_append(Vector** vector, void* data){
    if (
        vector == NULL || *vector == NULL ||
        data == NULL
    )
        return -1;
    
    (*vector)->size += 1;
    if ((*vector)->size >= (*vector)->capacity){ 
        if(!(*vector = vector_resize(*vector, (*vector)->capacity * 2))) return -1;
    }
    memcpy(
        (*vector)->vector + (((*vector)->size - 1) * (*vector)->data_type_size), 
        data, (*vector)->data_type_size
    );
    return 0;
}

void* vector_get(Vector* vector, size_t index){
    if (
        vector == NULL || 
        index >= vector->size
    )
        return NULL;

    return (vector->vector + (index * vector->data_type_size));
}

int vector_remove(Vector** vector, size_t index){
    if (
        vector == NULL || *vector == NULL || 
        (*vector)->size <= index
    )
        return -1;
    
    if ( index != ((*vector)->size - 1))
        memmove(
            (*vector)->vector + (index * (*vector)->data_type_size), 
            (*vector)->vector + ((index + 1) * (*vector)->data_type_size), 
            (((*vector)->size - 1) - index) * (*vector)->data_type_size
        );
    
    (*vector)->size -= 1;
    if ((*vector)->size < (*vector)->capacity * 0.25 && (*vector)->capacity > VEC_BASE_SIZE ) {
        if(!(*vector = vector_resize(*vector, (size_t)(*vector)->capacity / 2))) return -1;
    }
    
    return 0;
}

size_t vector_get_size(Vector* vector){
    return vector->size;
}

size_t vector_get_data_type_size(Vector* vector){
    return vector->data_type_size;
}


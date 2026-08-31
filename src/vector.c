#include "vector.h"

enum{
    VEC_BASE_SIZE = 16
};

struct Vector{
    size_t  capacity;
    size_t  size;
    size_t  data_type_size;
    void    *vector;
};

Vector* vector_construct(
    size_t data_type_size
){
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

int vector_destroy(
    Vector  *vector
){
    if (!vector)
        return VEC_INVALID_PARAM;
    if (vector->vector != NULL ) free(vector->vector);
    free(vector);
    return 0;
}

int vector_find(
    Vector    *vector, 
    void      *data,
    size_t    data_size
){
    if (
        !vector ||
        !data ||
        !vector->vector
    ) return VEC_INVALID_PARAM;

    if (
        data_size != vector->data_type_size
    ) return VEC_INVALID_PARAM;

    for(
        int idx = 0;
        idx < vector->size;
        idx ++
    ){
        if (
            !memcmp(
                vector_get(
                    vector,
                    idx
                ),
                data,
                data_size
            )
        ) return idx;
    }

    return -1;
}

int vector_append(
    Vector  **vector, 
    void    *data,
    size_t  data_size
){
    if (
        !vector || !(*vector) ||
        !data
    ) return VEC_INVALID_PARAM;

    if (
        (*vector)->data_type_size != data_size
    ) return VEC_TYPE_MISMATCH; 
    
    (*vector)->size += 1;
    if ((*vector)->size >= (*vector)->capacity){ 
        if(!(*vector = vector_resize(*vector, (*vector)->capacity * 2))) 
            return VEC_RESIZE_FAILED;
    }
    memcpy(
        (*vector)->vector + (((*vector)->size - 1) * (*vector)->data_type_size), 
        data, (*vector)->data_type_size
    );
    return 0;
}

int vector_concat(
    Vector  **dest_vector,
    Vector  *src_vector
){
    if (
        !dest_vector || !(*dest_vector) ||
        !src_vector
    ) return VEC_INVALID_PARAM;

    if (
        (*dest_vector)->data_type_size != 
        src_vector->data_type_size
    ) return VEC_TYPE_MISMATCH;

    for(
        int idx = 0; 
        idx < src_vector->size ; 
        idx++
    ){
        int err = vector_append(
            dest_vector,
            vector_get(
                src_vector, idx
            ),
            src_vector->data_type_size
        );

        if (
            err 
        ) return err;
    }

    return 0;
}

void* vector_get(
    Vector  *vector, 
    size_t  index
){
    if (
        !vector || 
        index >= vector->size
    )
        return NULL;

    return (vector->vector + (index * vector->data_type_size));
}

int vector_remove(
    Vector  **vector, 
    size_t  index
){
    if (
        !vector || !(*vector) || 
        (*vector)->size <= index
    )
        return VEC_INVALID_PARAM;
    
    if ( index != ((*vector)->size - 1))
        memmove(
            (*vector)->vector + (index * (*vector)->data_type_size), 
            (*vector)->vector + ((index + 1) * (*vector)->data_type_size), 
            (((*vector)->size - 1) - index) * (*vector)->data_type_size
        );
    
    (*vector)->size -= 1;
    if (
        (*vector)->size < (*vector)->capacity * 0.25 && 
        (*vector)->capacity > VEC_BASE_SIZE 
    ) {
        if(!(*vector = vector_resize(*vector, (size_t)(*vector)->capacity / 2))) 
            return VEC_RESIZE_FAILED;
    }
    
    return 0;
}

size_t vector_get_size(
    Vector *vector
){
    return vector->size;
}

size_t vector_get_data_type_size(
    Vector *vector
){
    return vector->data_type_size;
}

static Vector* vector_resize(
    Vector  *vector, 
    size_t  new_capacity
){
    assert(vector);
    assert(new_capacity > 0);
    
    vector->capacity = new_capacity;
    void *new_vec = (void*)reallocarray(
        vector->vector, 
        vector->capacity, 
        vector->data_type_size
    );
    if (new_vec == NULL) 
        return NULL;

    vector->vector = new_vec;
    return vector;
}
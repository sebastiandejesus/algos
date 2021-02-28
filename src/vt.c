#include <assert.h>
#include <stdlib.h>

#include "vt.h"

struct _vector {
    size_t size;
    size_t index;
    size_t capacity;
    #ifdef SYNC
        pthread_mutex_t mutex;
    #endif
    vt_Vector_Element *list;
};

/**
 * @brief Destroy a vector element.
 * 
 * @param vt    pointer to a vector element.
 * @param dtor  element destructor function pointer.
 */
static void _destroy_element(vt_Vector_Element *elem, vt_ElemDtor dtor);

/**
 * @brief Double the capacity of the vector.
 *
 * @param vt  pointer to a pointer to vector.
 */ 
static void _grow_vector(vt_Vector **vt);

/**
 * @brief Recursive sorting function.
 *
 * @param list  pointer to list elements.
 * @param low   the index to the beginning of the array.
 * @param high  the index to the end of the array.
 * @param comp  pointer to compare function.
 */ 
static void _sort(vt_Vector_Element *list, size_t low, size_t high,
                  vt_ElemCompare comp);

/**
 * @brief Merge two vectors.
 *
 * @param list  pointer to pointer of list elements.
 * @param low   the index to the beginning of the array.
 * @param mid   the index to the middle of the array.
 * @param high  the index to the end of the array.
 * @param comp  pointer to compare function.
 */ 
static void _merge(vt_Vector_Element **list, size_t low, size_t mid,
                   size_t high, vt_ElemCompare comp);

vt_Vector *vt_init(void)
{
    vt_Vector *vt = (vt_Vector*)calloc(1, sizeof *vt);
    assert(vt);
    vt->size = 0;
    vt->index = 0;
    vt->capacity = vt_INITIAL_VECTOR_CAPACITY;
    #ifdef SYNC
        if (pthread_mutex_init(&vt->mutex, NULL) != 0) {
            int errnum = errno;
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s error: unable to initialize mutex. errorcode: %d\n",
                        LOCATION, errnum);
            #endif
            free(vt);
            return NULL;
        }
    #endif
    vt->list = (vt_Vector_Element*)calloc(vt_INITIAL_VECTOR_CAPACITY,
                sizeof *vt->list);
    assert(vt->list);
    return vt;
}

void vt_destroy(vt_Vector *vt, vt_ElemDtor dtor)
{
    assert(vt);
    size_t i;
    for (i = 0; i < vt->size; ++i) {
        _destroy_element(vt->list[i], dtor);
    }
    #ifdef SYNC
        if (pthread_mutex_destroy(&vt->mutex) != 0) {
            int errnum = errno;
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s error: unable to destroy mutex. errorcode: %d\n",
                        LOCATION, errnum);
            #endif
        }
    #endif
    free(vt);
}

void vt_add(vt_Vector *vt, const vt_Vector_Element elem)
{
    assert(vt);
    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif

    if (vt->size == vt->capacity)
        _grow_vector(&vt);

    vt->list[vt->index++] = CONST_CAST(vt_Vector_Element, elem);
    vt->size++;

    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif
}

void vt_add_at(vt_Vector *vt, const vt_Vector_Element elem, size_t pos)
{
    assert(vt);
    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif

    if (pos <= vt->size) {
        if (vt->size == vt->capacity)
            _grow_vector(&vt);
    
        vt->list[pos] = CONST_CAST(vt_Vector_Element, elem);
        vt->size++;
    } else {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s error: pos is out-of-bounds\n", LOCATION);
        #endif
    }

    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif
}

const vt_Vector_Element vt_get(vt_Vector *vt)
{
    assert(vt);

    if (vt_isempty(vt)) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s error: vector is empty\n", LOCATION);
        #endif
        return NULL;
    }

    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif
    const vt_Vector_Element elem = vt->list[vt->index-1];
    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif
    return elem;
}

const vt_Vector_Element vt_get_at(vt_Vector *vt, size_t pos)
{
    vt_Vector_Element elem = NULL;
    assert(vt);

    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif

    if (pos >= 0 && pos < vt->size) {
        elem = vt->list[pos];
    } else {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s error: pos is out-of-bounds\n", LOCATION);
        #endif
    }

    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif
    return elem;
}

void vt_remove(vt_Vector *vt, vt_ElemDtor dtor)
{
    assert(vt);
    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif

    if (dtor) {
        dtor(vt->list[vt->index-1]);
    } else {
        free(vt->list[vt->index-1]);
        vt->list[vt->index-1] = NULL;
    }

    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif
}

void vt_remote_at(vt_Vector *vt, size_t pos, vt_ElemDtor dtor)
{
    assert(vt);
    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif

    if (pos >= 0 && pos < vt->size) {
        if (dtor) {
            dtor(vt->list[pos]);
        } else {
            free(vt->list[pos]);
            vt->list[pos] = NULL;
        }
    }

    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif
}

void vt_sort(vt_Vector *vt, vt_ElemCompare comp)
{
    assert(vt);
    assert(comp);
    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif
    _sort(vt->list, 0, vt->size, comp);
    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif
}

void vt_print(vt_Vector *vt, vt_ElemPrint print)
{
    assert(vt);
    assert(print);
    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif
    for (size_t i = 0; i < vt->size; ++i) {
        print(vt->list[i]);
    }
    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif
}

size_t vt_getsize(vt_Vector *vt)
{
    assert(vt);
    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif
    size_t size = vt->size;
    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif
    return size;
}

bool vt_isempty(vt_Vector *vt)
{
    assert(vt);
    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif
    bool empty = (vt->size == 0) ? true : false;
    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif
    return empty;
}

static void _destroy_element(vt_Vector_Element *elem, vt_ElemDtor dtor)
{
    if (dtor)
        dtor(elem);
    else
        free(elem);
}

static void _grow_vector(vt_Vector **vt)
{
    (*vt)->list = realloc((*vt)->list, (*vt)->capacity*2 * sizeof *((*vt)->list));
    assert((*vt)->list);
    (*vt)->capacity *= 2;
}

static void _sort(vt_Vector_Element *list, size_t low, size_t high,
                  vt_ElemCompare comp)
{
    if (low < high) {
        size_t mid = (high - low) / 2;
        _sort(list, low, mid+1, comp);
        _sort(list, mid+1, high, comp);
        _merge(&list, low, mid+1, high, comp);
    }
}

static void _merge(vt_Vector_Element **list, size_t low, size_t mid,
                   size_t high, vt_ElemCompare comp)
{
    size_t i, j, k;
    vt_Vector_Element *l1 = (*list)[low];
    vt_Vector_Element *l2 = (*list)[mid];

    for (i = 0, j = 0, k = 0; i < mid && j < high; ++k) {
        if (comp(l1[i], l2[j]) < 1)
            (*list)[k] = l1[i++];
        else
            (*list)[k] = l2[j++];
    }

    while (i < mid)
        (*list)[k++] = l1[i++];

    while (j < high)
        (*list)[k++] = l2[j++];
}

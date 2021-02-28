#ifndef VT_H
#define VT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

#include "constants.h"

#if VECTOR_CAPACITY > 50
#define vt_INITIAL_VECTOR_CAPACITY VECTOR_CAPACITY
#else
#define vt_INITIAL_VECTOR_CAPACITY 50
#endif

/**
 * @brief Vector abstract data type.
 */
typedef struct _vector vt_Vector;

/**
 * @brief Vector element data type.
 */
typedef void* vt_Vector_Element;

/**
 * @brief Vector element destructor function pointer type. 
 */
typedef void (*vt_ElemDtor)(vt_Vector_Element); 

/**
 * @brief Vector element compare function pointer type.
 */
typedef int (*vt_ElemCompare)(const vt_Vector_Element,
                              const vt_Vector_Element); 

/**
 * @brief Vector element print function pointer type.
 */
typedef void (*vt_ElemPrint)(const vt_Vector_Element);

/**
 * @brief Cast to an vt_Vector_Element type.
 *
 * @param e  object to convert.
 * @return an vt_Vector_Element type.
 */
#define vt_convert_to_vector_element(e) (vt_Vector_Element)e

/**
 * @brief Cast from vt_Vector_Element to original type.
 *
 * @param t  original type to convert to.
 * @param e  vt_Vector_Element object to convert.
 * @return the original type.
 */
#define vt_return_from_vector_element(t,e) (t)e

/**
 * @brief Initialize a vector.
 *
 * @return an vector object.
 */
extern LIB_EXPORT vt_Vector *vt_init(void) NOTHROW;

/**
 * @brief Destroy a vector.
 * 
 * @param vt    pointer to a vector.
 * @param dtor  element destructor function pointer.
 */
extern LIB_EXPORT void vt_destroy(vt_Vector *vt, vt_ElemDtor dtor);

/**
 * @brief Add an element to the end of the vector.
 *
 * @param vt    pointer to a vector.
 * @param elem  the vector element to add.
 */
extern LIB_EXPORT void vt_add(vt_Vector *vt,
                              const vt_Vector_Element elem) NOTHROW;

/**
 * @brief Add an element to the vector at a specific postion.
 *
 * @param vt    pointer to a vector.
 * @param elem  the vector element to add.
 * @param pos   the vector position to add the element.
 */
extern LIB_EXPORT void vt_add_at(vt_Vector *vt,
                                 const vt_Vector_Element elem,
                                 size_t pos) NOTHROW;

/**
 * @brief Get an element to the end of the vector.
 *
 * @param vt  pointer to a vector.
 * @return the element at the end of the vector.
 */
extern LIB_EXPORT const vt_Vector_Element vt_get(vt_Vector *vt) NOTHROW;

/**
 * @brief Get an element in the vector at a specific postion.
 *
 * @param vt   pointer to a vector.
 * @param pos  the vector position of the element.
 * @return the vector element at the specified position. 
 */
extern LIB_EXPORT const vt_Vector_Element vt_get_at(vt_Vector *vt,
                                                       size_t pos) NOTHROW;

/**
 * @brief Remove an element at the end of the vector.
 *
 * @param vt    pointer to a vector.
 * @param dtor  element destructor function pointer.
 */
extern LIB_EXPORT void vt_remove(vt_Vector *vt, vt_ElemDtor dtor);

/**
 * @brief Remove an element in the vector at a specific postion.
 *
 * @param vt    pointer to a vector.
 * @param pos   the vector position to remove the element.
 * @param dtor  element destructor function pointer.
 */
extern LIB_EXPORT void vt_remove_at(vt_Vector *vt, size_t pos,
                                    vt_ElemDtor dtor);

/**
 * @brief Sort vector elements.
 * @param vt    pointer to a vector.
 * @param comp  element compare function pointer.
 */
extern LIB_EXPORT void vt_sort(vt_Vector *vt, vt_ElemCompare comp);

/**
 * @brief Print vector elements.
 * @param vt     pointer to a vector.
 * @param print  element print function pointer.
 */
extern LIB_EXPORT void vt_print(vt_Vector *vt, vt_ElemPrint print);

/**
 * @brief Return vector size.
 *
 * @param vt  pointer to a vector.
 * @return the size of the vector.
 */
extern LIB_EXPORT size_t vt_getsize(vt_Vector *vt) NOTHROW;

/**
 * @brief Check if vector is empty.
 *
 * @param vt  pointer to a vector.
 * @return true if the vector is empty, false if not.
 */
extern LIB_EXPORT bool vt_isempty(vt_Vector *vt) NOTHROW;

#ifdef __cplusplus
}
#endif

#endif /* VT_H */

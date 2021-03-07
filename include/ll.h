#ifndef LL_H
#define LL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

#include "constants.h"

/* enumeration types */
typedef enum {ll_SINGLY, ll_DOUBLY, ll_CIRCLY} ll_ListType;

/* adt types */
typedef struct _linkedlist ll_LinkedList;

/* misc. function pointer types */
typedef void (*ll_ElemDtor)(void*);
typedef int (*ll_ElemCompare)(const void*, const void*);
typedef void (*ll_ElemPrint)(const void*);

/* linkedlist ctor & dtor */
extern LIB_EXPORT ll_LinkedList *ll_init(ll_ListType type) NOTHROW;
extern LIB_EXPORT void ll_destroy(ll_LinkedList *ll, ll_ElemDtor dtor);

/* insert subroutines */
extern LIB_EXPORT int ll_insert(ll_LinkedList *ll, const void *elem) NOTHROW;
extern LIB_EXPORT int ll_insert_atend(ll_LinkedList *ll, const void *elem) NOTHROW;
extern LIB_EXPORT int ll_insert_before(ll_LinkedList *ll, const void *elem, const void *new_elem) NOTHROW;
extern LIB_EXPORT int ll_insert_after(ll_LinkedList *ll, const void *elem, const void *new_elem) NOTHROW;

/* get subroutines */
extern LIB_EXPORT void *ll_get(const ll_LinkedList *ll) NOTHROW;
extern LIB_EXPORT void *ll_get_atend(const ll_LinkedList *ll) NOTHROW;
extern LIB_EXPORT void *ll_get_before(const ll_LinkedList *ll, const void *elem) NOTHROW;
extern LIB_EXPORT void *ll_get_after(const ll_LinkedList *ll, const void *elem) NOTHROW;

/* delete subroutines */
extern LIB_EXPORT void ll_delete(ll_LinkedList *ll, const void *elem, ll_ElemDtor dtor);
extern LIB_EXPORT void ll_delete_atend(ll_LinkedList *ll, ll_ElemDtor dtor);
extern LIB_EXPORT void ll_delete_before(ll_LinkedList *ll, const void *elem, ll_ElemDtor dtor);
extern LIB_EXPORT void ll_delete_after(ll_LinkedList *ll, const void *elem, ll_ElemDtor dtor);

/* other subroutines */
extern LIB_EXPORT int ll_insert_elementatpos(ll_LinkedList *ll, const void *elem, size_t pos) NOTHROW;
extern LIB_EXPORT void *ll_get_elementatpos(const ll_LinkedList *ll, size_t pos) NOTHROW;
extern LIB_EXPORT void ll_delete_elementatpos(ll_LinkedList *ll, size_t pos, ll_ElemDtor dtor);
extern LIB_EXPORT bool ll_islinkedlistempty(const ll_LinkedList *ll) NOTHROW;
extern LIB_EXPORT size_t ll_getlinkedlistsize(const ll_LinkedList *ll) NOTHROW;
extern LIB_EXPORT int ll_getlinkedlisttype(const ll_LinkedList *ll) NOTHROW;
extern LIB_EXPORT bool ll_search(const ll_LinkedList *ll, const void *elem) NOTHROW;
extern LIB_EXPORT bool ll_search_reverse(const ll_LinkedList *ll, const void *elem) NOTHROW;
extern LIB_EXPORT void ll_reverse(ll_LinkedList *ll) NOTHROW;
extern LIB_EXPORT void ll_sort(ll_LinkedList *ll, size_t min, size_t max, ll_ElemCompare comp);
extern LIB_EXPORT int ll_exchange(ll_LinkedList *ll, const void *elem1, const void *elem2) NOTHROW;
extern LIB_EXPORT void ll_print(const ll_LinkedList *ll, ll_ElemPrint print);
extern LIB_EXPORT void ll_print_reverse(const ll_LinkedList *ll, ll_ElemPrint print);

#ifdef __cplusplus
}
#endif

#endif /* LL_H */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ll.h"

/**
 * singly node type.
 */
typedef struct _sn {
    void *elem;
    struct _sn *next;
} SinglyNode;

/**
 * doubly node type.
 */
typedef struct _dn {
    void *elem;
    struct _dn *next;
    struct _dn *prev;
} DoublyNode;

/**
 * generic linkedlist type.
 */
struct _linkedlist {
    ll_ListType type;
    size_t size;
    #ifdef SYNC
        pthread_mutex_t mutex;
    #endif
    union {
        SinglyNode *s_head;
        DoublyNode *d_head;
    };
    union {
        SinglyNode *s_tail;
        DoublyNode *d_tail;
    };
};

/**
 * default destructor for linkedlist element.
 */
static inline void _defaultdtor(void *elem)
{ 
    if (NULL != elem)
        free(elem);
}

/**
 * create a singly node (heap alloc).
 */
static SinglyNode *_init_singlynode(const void *elem);

/**
 * create a doubly node (heap alloc).
 */
static DoublyNode *_init_doublynode(const void *elem);

/**
 * destroy singly node.
 */
static void _destroy_singlynode(SinglyNode *node, ll_ElemDtor dtor);

/**
 * destroy doubly node.
 */
static void _destroy_doublynode(DoublyNode *node, ll_ElemDtor dtor);

/**
 * merge 2 sorted linkedlists into original.
 */
static void _merge(ll_LinkedList *ll, size_t min, size_t mid, size_t max, ll_ElemCompare comp);

/**
 * recursive function for printing (singly, circly) linkedlist
 * in reverse order.
 */
static void _print_reverse(SinglyNode *start, SinglyNode *end, ll_ElemPrint print);

/**
 * recursive function for searching (singly, circly) linkedlist
 * in reverse order.
 */
static bool _search_reverse(SinglyNode *start, SinglyNode *end, const void *elem);


ll_LinkedList *ll_init(ll_ListType type)
{
    ll_LinkedList *ll = NULL;
    switch (type) {
        case ll_CIRCLY:
            ll = (ll_LinkedList*)calloc(1, sizeof *ll); 
            assert(ll);
            ll->type = type;
            ll->size = INIT_LL_SIZE_VAL;
            #ifdef SYNC
                if (pthread_mutex_init(&ll->mutex, NULL) != 0) {
                    int errnum = errno;
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: unable to initialize mutex. errorcode: %d\n",
                                FUNC, errnum);
                    #endif
                    free(ll);
                    return NULL;
                }
            #endif
            ll->s_head = _init_singlynode(NULL);
            ll->s_tail = _init_singlynode(NULL);
            ll->s_head->next = ll->s_tail;
            ll->s_tail->next = ll->s_head;
            break;
        case ll_DOUBLY:
            ll = (ll_LinkedList*)calloc(1, sizeof *ll); 
            assert(ll);
            ll->type = type;
            ll->size = INIT_LL_SIZE_VAL;
            #ifdef SYNC
                if (pthread_mutex_init(&ll->mutex, NULL) != 0) {
                    int errnum = errno;
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: unable to initialize mutex. errorcode: %d\n",
                                FUNC, errnum);
                    #endif
                    free(ll);
                    return NULL;
                }
            #endif
            ll->d_head = _init_doublynode(NULL);
            ll->d_tail = _init_doublynode(NULL);
            ll->d_head->next = ll->d_tail;
            ll->d_head->prev = ll->d_head;
            ll->d_tail->next = ll->d_tail;
            ll->d_tail->prev = ll->d_head;
            break;
        case ll_SINGLY:
            ll = (ll_LinkedList*)calloc(1, sizeof *ll); 
            assert(ll);
            ll->type = type;
            ll->size = INIT_LL_SIZE_VAL;
            #ifdef SYNC
                if (pthread_mutex_init(&ll->mutex, NULL) != 0) {
                    int errnum = errno;
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: unable to initialize mutex. errorcode: %d\n",
                                FUNC, errnum);
                    #endif
                    free(ll);
                    return NULL;
                }
            #endif
            ll->s_head = _init_singlynode(NULL);
            ll->s_tail = _init_singlynode(NULL);
            ll->s_head->next = ll->s_tail;
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
    return ll;
}

void ll_destroy(ll_LinkedList *ll, ll_ElemDtor dtor)
{
    assert(ll);

    switch (ll->type) {
        case ll_CIRCLY:
            {
                SinglyNode *curnode = ll->s_head->next;
                SinglyNode *nextnode = NULL;
                while (curnode && curnode->next != ll->s_head) {
                    nextnode = curnode->next;
                    _destroy_singlynode(curnode, dtor);
                    curnode = nextnode;
                }
                _destroy_singlynode(ll->s_head, dtor);
                _destroy_singlynode(ll->s_tail, dtor);
            }
            break;
        case ll_DOUBLY:
            {
                DoublyNode *curnode = ll->d_head->next;
                DoublyNode *nextnode = NULL;
                while (curnode && curnode != curnode->next) {
                    nextnode = curnode->next;
                    _destroy_doublynode(curnode, dtor);
                    curnode = nextnode;
                }
                _destroy_doublynode(ll->d_head, dtor);
                _destroy_doublynode(ll->d_tail, dtor);
            }
            break;
        case ll_SINGLY:
            {
                SinglyNode *curnode = ll->s_head->next;
                SinglyNode *nextnode = NULL;
                while (curnode && curnode != curnode->next) {
                    nextnode = curnode->next;
                    _destroy_singlynode(curnode, dtor);
                    curnode = nextnode;
                }
                _destroy_singlynode(ll->s_head, dtor);
                _destroy_singlynode(ll->s_tail, dtor);
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            return;
    }
    #ifdef SYNC
        if (pthread_mutex_destroy(&ll->mutex) != 0) {
            int errnum = errno;
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: unable to destroy mutex. errorcode: %d\n",
                        FUNC, errnum);
            #endif
        }
    #endif
    free(ll);
    ll = NULL;
}

static SinglyNode *_init_singlynode(const void *elem)
{
    SinglyNode *node = (SinglyNode*)calloc(1, sizeof *node);       
    assert(node);
    node->elem = CONST_CAST(void*, elem);
    node->next = NULL;
    return node;
}

static DoublyNode *_init_doublynode(const void *elem)
{
    DoublyNode *node = (DoublyNode*)calloc(1, sizeof *node);       
    assert(node);
    node->elem = CONST_CAST(void*, elem);
    node->next = NULL;
    node->prev = NULL;
    return node;
}

static void _destroy_singlynode(SinglyNode *node, ll_ElemDtor dtor)
{
    if (dtor == NULL)
        dtor = _defaultdtor;

    dtor(node->elem);
    node->next = NULL;
    free(node);
    node = NULL;
}

static void _destroy_doublynode(DoublyNode *node, ll_ElemDtor dtor)
{
    if (dtor == NULL)
        dtor = _defaultdtor;

    dtor(node->elem);
    node->next = NULL;
    node->prev = NULL;
    free(node);
    node = NULL;
}

int ll_insert(ll_LinkedList *ll, const void *elem)
{
    assert(ll);

    switch (ll->type) {
        case ll_CIRCLY:
        case ll_SINGLY:
            {
                SinglyNode *node = _init_singlynode(elem);
                node->elem = CONST_CAST(void*, elem);

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                node->next = ll->s_head->next;
                ll->s_head->next = node;
                ll->size++;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                DoublyNode *node = _init_doublynode(elem);
                node->elem = CONST_CAST(void*, elem);

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                node->next = ll->d_head->next;
                node->prev = ll->d_head;
                ll->d_head->next->prev = node;
                ll->d_head->next = node;
                ll->size++;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
         default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            return ERROR;
    }
    return SUCCESS;
}

int ll_insert_atend(ll_LinkedList *ll, const void *elem)
{
    assert(ll);

    if (ll_islinkedlistempty(ll))
        return ll_insert(ll, elem);

    switch (ll->type) {
        case ll_SINGLY:
            {
                SinglyNode *node = _init_singlynode(elem);

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next != tmp->next->next)
                    tmp = tmp->next;

                node->elem = CONST_CAST(void*, elem);
                node->next = tmp->next;
                tmp->next = node;
                ll->size++;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                DoublyNode *node = _init_doublynode(elem);

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next != tmp->next->next)
                    tmp = tmp->next;

                node->elem = CONST_CAST(void*, elem);
                node->next = tmp->next;
                node->prev = tmp;
                tmp->next = node;
                ll->size++;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_CIRCLY:
            {
                SinglyNode *node = _init_singlynode(elem);

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != ll->s_head)
                    tmp = tmp->next;

                node->elem = CONST_CAST(void*, elem);
                node->next = tmp->next;
                tmp->next = node;
                ll->size++;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            return ERROR;
    }
    return SUCCESS;
}

int ll_insert_before(ll_LinkedList *ll, const void *elem, const void *new_elem)
{
    bool found = false;

    assert(ll);

    if (ll_islinkedlistempty(ll)) {
        int rc = ll_insert(ll, new_elem);
        if (SUCCESS == rc)
            return ll_insert(ll, elem);
        else
            return rc;
    }

    switch (ll->type) {
        case ll_SINGLY:
            {
                SinglyNode *node = _init_singlynode(elem);

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->next->elem == elem) {
                        found = true;
                        node->elem = CONST_CAST(void*, new_elem);
                        node->next = tmp->next;
                        tmp->next = node;
                        ll->size++;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                DoublyNode *node = _init_doublynode(elem);

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->next->elem == elem) {
                        found = true;
                        node->elem = CONST_CAST(void*, new_elem);
                        node->next = tmp->next;
                        node->prev = tmp;
                        tmp->next = node;
                        ll->size++;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_CIRCLY:
            {
                SinglyNode *node = _init_singlynode(elem);

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != ll->s_head) {
                    if (tmp->next->elem == elem) {
                        found = true;
                        node->elem = CONST_CAST(void*, new_elem);
                        node->next = tmp->next;
                        tmp->next = node;
                        ll->size++;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            return ERROR;
    }

    if (!found) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: linkedlist elem not found\n", FUNC);
        #endif
        return NOTFOUND;
    }
    return SUCCESS;
}

int ll_insert_after(ll_LinkedList *ll, const void *elem, const void *new_elem)
{
    bool found = false;

    assert(ll);

    if (ll_islinkedlistempty(ll)) {
        int rc = ll_insert(ll, elem);
        if (SUCCESS == rc)
            rc = ll_insert(ll, new_elem);
        return rc;
    }

    switch (ll->type) {
        case ll_SINGLY:
            {
                SinglyNode *node = _init_singlynode(elem);

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->elem == elem) {
                        found = true;
                        node->elem = CONST_CAST(void*, new_elem);
                        node->next = tmp->next;
                        tmp->next = node;
                        ll->size++;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                DoublyNode *node = _init_doublynode(elem);

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->elem == elem) {
                        found = true;
                        node->elem = CONST_CAST(void*, new_elem);
                        node->next = tmp->next;
                        node->prev = tmp;
                        tmp->next = node;
                        ll->size++;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_CIRCLY:
            {
                SinglyNode *node = _init_singlynode(elem);

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != ll->s_head) {
                    if (tmp->elem == elem) {
                        found = true;
                        node->elem = CONST_CAST(void*, new_elem);
                        node->next = tmp->next;
                        tmp->next = node;
                        ll->size++;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            return ERROR;
    }

    if (!found) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: linkedlist elem not found\n", FUNC);
        #endif
        return NOTFOUND;
    }
    return SUCCESS;
}

void *ll_get(const ll_LinkedList *ll)
{
    void *elem = NULL;

    assert(ll);

    if (ll_islinkedlistempty(ll)) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: linkedlist is empty\n", FUNC);
        #endif
        return NULL;
    }

    switch (ll->type) {
        case ll_CIRCLY:
        case ll_SINGLY:
            #ifdef SYNC
                ll_LOCK(&ll->mutex);
            #endif
            elem = ll->s_head->next->elem;
            #ifdef SYNC
                ll_UNLOCK(&ll->mutex);
            #endif
            break;
        case ll_DOUBLY:
            #ifdef SYNC
                ll_LOCK(&ll->mutex);
            #endif
            elem = ll->d_head->next->elem;
            #ifdef SYNC
                ll_UNLOCK(&ll->mutex);
            #endif
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
    return elem;
}

void *ll_get_atend(const ll_LinkedList *ll)
{
    void *elem = NULL;

    assert(ll);

    if (ll_islinkedlistempty(ll)) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: linkedlist is empty\n", FUNC);
        #endif
        return NULL;
    }

    switch (ll->type) {
        case ll_CIRCLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != ll->s_head)
                    tmp = tmp->next;
                elem = tmp->elem;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next != tmp->next->next)
                    tmp = tmp->next;
                elem = tmp->elem;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_SINGLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next != tmp->next->next)
                    tmp = tmp->next;
                elem = tmp->elem;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
    return elem;
}

void *ll_get_before(const ll_LinkedList *ll, const void *elem)
{
    void *element = NULL;

    assert(ll);

    if (ll_islinkedlistempty(ll)) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: linkedlist is empty\n", FUNC);
        #endif
        return NULL;
    }

    switch (ll->type) {
        case ll_CIRCLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != ll->s_head) {
                    if (tmp->next->elem == elem) {
                        element = tmp->elem;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->next->elem == elem) {
                        element = tmp->elem;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_SINGLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->next->elem == elem) {
                        element = tmp->elem;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
    return element;
}

void *ll_get_after(const ll_LinkedList *ll, const void *elem)
{
    void *element = NULL;

    assert(ll);

    if (ll_islinkedlistempty(ll)) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: linkedlist is empty\n", FUNC);
        #endif
        return NULL;
    }

    switch (ll->type) {
        case ll_SINGLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->elem == elem) {
                        element = tmp->next->elem;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->elem == elem) {
                        element = tmp->next->elem;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_CIRCLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != ll->s_head) {
                    if (tmp->elem == elem) {
                        element = tmp->next->elem;
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
    return element;
}

void ll_delete(ll_LinkedList *ll, const void *elem, ll_ElemDtor dtor)
{
    assert(ll);

    switch (ll->type) {
        case ll_CIRCLY:
            {
                SinglyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != ll->s_head) {
                    if (tmp->next->elem == elem) {
                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        _destroy_singlynode(node, dtor);
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                DoublyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->next->elem == elem) {
                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        tmp->next->next->prev = tmp;
                        _destroy_doublynode(node, dtor);
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_SINGLY:
            {
                SinglyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->next->elem == elem) {
                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        _destroy_singlynode(node, dtor);
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
}

void ll_delete_atend(ll_LinkedList *ll, ll_ElemDtor dtor)
{
    assert(ll);

    switch (ll->type) {
        case ll_CIRCLY:
            {
                SinglyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != ll->s_head)
                    tmp = tmp->next;

                node = tmp->next;
                tmp->next = tmp->next->next;
 
                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif

                _destroy_singlynode(node, dtor);

            }
            break;
        case ll_DOUBLY:
            {
                DoublyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next->next != tmp->next->next->next)
                    tmp = tmp->next;

                node = tmp->next;
                tmp->next = tmp->next->next;
                tmp->next->next->prev = tmp;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif

                _destroy_doublynode(node, dtor);
            }
            break;
        case ll_SINGLY:
            {
                SinglyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != tmp->next->next->next)
                    tmp = tmp->next;

                node = tmp->next;
                tmp->next = tmp->next->next;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif

                _destroy_singlynode(node, dtor);
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
}

void ll_delete_before(ll_LinkedList *ll, const void *elem, ll_ElemDtor dtor)
{
    assert(ll);

    switch (ll->type) {
        case ll_CIRCLY:
            {
                SinglyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != ll->s_head)
                    tmp = tmp->next;

                node = tmp->next;
                tmp->next = tmp->next->next;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif

                _destroy_singlynode(node, dtor);
            }
            break;
        case ll_DOUBLY:
            {
                DoublyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next->next != tmp->next->next->next)
                    tmp = tmp->next;

                node = tmp->next;
                tmp->next = tmp->next->next;
                tmp->next->next->prev = tmp;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif

                _destroy_doublynode(node, dtor);
            }
            break;
        case ll_SINGLY:
            {
                SinglyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != tmp->next->next->next)
                    tmp = tmp->next;

                node = tmp->next;
                tmp->next = tmp->next->next;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif

                _destroy_singlynode(node, dtor);
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
}

void ll_delete_after(ll_LinkedList *ll, const void *elem, ll_ElemDtor dtor)
{
    assert(ll);

    switch (ll->type) {
        case ll_CIRCLY:
            {
                SinglyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next != ll->s_head) {
                    if (tmp->elem == elem) {
                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        _destroy_singlynode(node, dtor);
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                DoublyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->elem == elem) {
                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        tmp->next->next->prev = tmp;
                        _destroy_doublynode(node, dtor);
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_SINGLY:
            {
                SinglyNode *node = NULL;

                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->elem == elem) {
                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        _destroy_singlynode(node, dtor);
                        break;
                    }
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
}

int ll_insert_elementatpos(ll_LinkedList *ll, const void *elem, size_t pos)
{
    int rc = ERROR;

    assert(ll);

    switch (ll->type) {
        case ll_CIRCLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                if (pos > ll->size) {
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: pos is too high\n", FUNC);
                    #endif
                } else {
                    SinglyNode *tmp = ll->s_head;
                    SinglyNode *node = _init_singlynode(elem);

                    if (pos == 0) {
                        node->next = tmp->next;
                        tmp->next = node;
                    } else {
                        while (--pos && tmp->next != ll->s_head)
                            tmp = tmp->next;

                        if (tmp->next == ll->s_head)
                            return rc;

                        node->next = tmp->next;
                        tmp->next = node;
                    }
                    ll->size++;
                    rc = SUCCESS;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                if (pos > ll->size) {
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: pos is too high\n", FUNC);
                    #endif
                } else {
                    DoublyNode *tmp = ll->d_head;
                    DoublyNode *node = _init_doublynode(elem);

                    if (pos == 0) {
                        node->next = tmp->next;
                        node->prev = tmp;
                        tmp->next->prev = node;
                        tmp->next = node;
                    } else {
                        while (--pos && tmp != tmp->next)
                            tmp = tmp->next;

                        if (tmp == tmp->next)
                            return rc;

                        node->next = tmp->next;
                        node->prev = tmp;
                        tmp->next->prev = node;
                        tmp->next = node;
                    }
                    ll->size++;
                    rc = SUCCESS;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_SINGLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                if (pos > ll->size) {
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: pos is too high\n", FUNC);
                    #endif
                } else {
                    SinglyNode *tmp = ll->s_head;
                    SinglyNode *node = _init_singlynode(elem);

                    if (pos == 0) {
                        node->next = tmp->next;
                        tmp->next = node;
                    } else {
                        while (--pos && tmp != tmp->next)
                            tmp = tmp->next;

                        if (tmp == tmp->next)
                            return rc;

                        node->next = tmp->next;
                        tmp->next = node;
                    }
                    ll->size++;
                    rc = SUCCESS;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
    return rc;
}

void *ll_get_elementatpos(const ll_LinkedList *ll, size_t pos)
{
    void *elem = NULL;

    assert(ll);

    if (ll_islinkedlistempty(ll)) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: linkedlist is empty\n", FUNC);
        #endif
        return NULL;
    }

    switch (ll->type) {
        case ll_CIRCLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                if (pos >= ll->size) {
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: pos is out-of-bounds\n", FUNC);
                    #endif
                } else {
                    SinglyNode *tmp = ll->s_head;

                    if (pos == 0) {
                        elem = tmp->next->elem;
                    } else {
                        while (--pos && tmp->next != ll->s_head)
                            tmp = tmp->next;

                        if (tmp->next == ll->s_head)
                            return NULL;

                        elem = tmp->elem;
                    }
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                if (pos >= ll->size) {
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: pos is out-of-bounds\n", FUNC);
                    #endif
                } else {
                    DoublyNode *tmp = ll->d_head;

                    if (pos == 0) {
                        elem = tmp->next->elem;
                    } else {
                        while (--pos && tmp != tmp->next)
                            tmp = tmp->next;

                        if (tmp == tmp->next)
                            return NULL;

                        elem = tmp->elem;
                    }
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_SINGLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                if (pos >= ll->size) {
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: pos is out-of-bounds\n", FUNC);
                    #endif
                } else {
                    SinglyNode *tmp = ll->s_head;

                    if (pos == 0) {
                        elem = tmp->next->elem;
                    } else {
                        while (--pos && tmp != tmp->next)
                            tmp = tmp->next;

                        if (tmp == tmp->next)
                            return NULL;

                        elem = tmp->elem;
                    }
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
    return elem;
}

void ll_delete_elementatpos(ll_LinkedList *ll, size_t pos, ll_ElemDtor dtor)
{
    assert(ll);

    if (ll_islinkedlistempty(ll)) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: linkedlist is empty\n", FUNC);
        #endif
        return;
    }

    switch (ll->type) {
        case ll_CIRCLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                if (pos >= ll->size) {
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: pos is out-of-bounds\n", FUNC);
                    #endif
                } else {
                    SinglyNode *tmp = ll->s_head;
                    SinglyNode *node = NULL;

                    if (pos == 0) {
                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        _destroy_singlynode(node, dtor);
                    } else {
                        while (--pos && tmp->next != ll->s_head)
                            tmp = tmp->next;

                        if (tmp->next != ll->s_head)
                            return;

                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        _destroy_singlynode(node, dtor);
                    }
                    ll->size--;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                if (pos >= ll->size) {
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: pos is out-of-bounds\n", FUNC);
                    #endif
                } else {
                    DoublyNode *tmp = ll->d_head;
                    DoublyNode *node = NULL;

                    if (pos == 0) {
                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        tmp->next->next->prev = tmp;
                        _destroy_doublynode(node, dtor);
                    } else {
                        while (--pos && tmp != tmp->next)
                            tmp = tmp->next;

                        if (tmp == tmp->next)
                            return;

                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        tmp->next->next->prev = tmp;
                        _destroy_doublynode(node, dtor);
                    }
                    ll->size--;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_SINGLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                if (pos >= ll->size) {
                    #ifdef ALGOS_DEBUG
                        fprintf(stderr, "%s() error: pos is out-of-bounds\n", FUNC);
                    #endif
                } else {
                    SinglyNode *tmp = ll->s_head;
                    SinglyNode *node = NULL;

                    if (pos == 0) {
                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        _destroy_singlynode(node, dtor);
                    } else {
                        while (--pos && tmp != tmp->next)
                            tmp = tmp->next;

                        if (tmp == tmp->next)
                            return;

                        node = tmp->next;
                        tmp->next = tmp->next->next;
                        _destroy_singlynode(node, dtor);
                    }
                    ll->size--;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
}

bool ll_islinkedlistempty(const ll_LinkedList *ll)
{
    bool empty = true;

    assert(ll);

    switch (ll->type) {
        case ll_CIRCLY:
        case ll_SINGLY:
            #ifdef SYNC
                ll_LOCK(&ll->mutex);
            #endif
            empty = (ll->s_head->next == ll->s_tail);
            #ifdef SYNC
                ll_UNLOCK(&ll->mutex);
            #endif
            break;
        case ll_DOUBLY:
            #ifdef SYNC
                ll_LOCK(&ll->mutex);
            #endif
            empty = (ll->d_head->next == ll->d_tail);
            #ifdef SYNC
                ll_UNLOCK(&ll->mutex);
            #endif
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
    return empty;
}

size_t ll_getlinkedlistsize(const ll_LinkedList *ll)
{
    size_t size = 0;

    assert(ll);

    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif
    size = ll->size;
    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif
    return size;
}

int ll_getlinkedlisttype(const ll_LinkedList *ll)
{
    assert(ll);
    return ll->type;  
}

bool ll_search(const ll_LinkedList *ll, const void *elem)
{
    bool found = false;

    assert(ll);

    if (ll_islinkedlistempty(ll))
        return false;

    switch (ll->type) {
        case ll_CIRCLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != ll->s_head) {
                    if (tmp->elem == elem)
                        found = true;
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->elem == elem)
                        found = true;
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_SINGLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next != tmp->next->next) {
                    if (tmp->elem == elem)
                        found = true;
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
    return found;
}

bool ll_search_reverse(const ll_LinkedList *ll, const void *elem)
{
    bool found = false;

    assert(ll);

    if (ll_islinkedlistempty(ll)) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: linkedlist is empty\n", FUNC);
        #endif
        return found;
    }

    switch (ll->type) {
        case ll_CIRCLY:
        case ll_SINGLY:
            #ifdef SYNC
                ll_LOCK(&ll->mutex);
            #endif
            found = _search_reverse(ll->s_head->next, ll->s_tail, elem);
            #ifdef SYNC
                ll_UNLOCK(&ll->mutex);
            #endif
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_tail->prev;
                while (tmp->prev != ll->d_head) {
                    if (tmp->elem == elem)
                        found = true;
                    tmp = tmp->prev;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
    return found;
}

void ll_reverse(ll_LinkedList *ll)
{
    assert(ll);

    if (ll_islinkedlistempty(ll)) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: linkedlist is empty\n", FUNC);
        #endif
        return;
    }

    switch (ll->type) {
        case ll_CIRCLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                SinglyNode *prev = ll->s_head;
                SinglyNode *node = NULL;

                while (tmp->next != ll->s_head) {
                    node = tmp->next; 
                    tmp->next = prev;
                    prev = tmp;
                    tmp = node;
                }
                node = ll->s_head;
                ll->s_head = ll->s_tail;
                ll->s_tail = node;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                DoublyNode *node = NULL;
                while (tmp->next != tmp->next->next) {
                    node = tmp->next;
                    tmp->next = tmp->prev;
                    tmp->prev = node;
                    tmp = node;
                }
                node = ll->d_head;
                ll->d_head = ll->d_tail;
                ll->d_tail = node;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_SINGLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                SinglyNode *prev = ll->s_head;
                SinglyNode *node = NULL;
                while (tmp->next != tmp->next->next) {
                    node = tmp->next; 
                    tmp->next = prev;
                    prev = tmp;
                    tmp = node;
                }
                node = ll->s_head;
                ll->s_head = ll->s_tail;
                ll->s_tail = node;

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
}

void ll_sort(ll_LinkedList *ll, size_t min, size_t max, ll_ElemCompare comp)
{
    assert(ll);
    assert(comp);

    if (min < max) {
        size_t mid = (min + max) / 2;
        ll_sort(ll, min, mid, comp);
        ll_sort(ll, mid+1, max, comp);
        _merge(ll, min, mid+1, max, comp);
    }
}


int ll_exchange(ll_LinkedList *ll, const void *elem1, const void *elem2)
{
    int rc = ERROR;

    assert(ll);

    if (ll_islinkedlistempty(ll)) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: linkedlist is empty\n", FUNC);
        #endif
        return rc;
    }

    switch (ll->type) {
        case ll_CIRCLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                SinglyNode *n1 = NULL;
                SinglyNode *n2 = NULL;
                while (tmp->next->next != ll->s_head) {
                    if (tmp->elem == elem1)
                        n1 = tmp;
                    else if (tmp->elem == elem2)
                        n2 = tmp;
                    tmp = tmp->next;
                }
                if (n1 != NULL && n2 != NULL) {
                    void *e = n1->elem;
                    n1->elem = n2->elem;
                    n2->elem = e;
                    rc = SUCCESS;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                DoublyNode *n1 = NULL;
                DoublyNode *n2 = NULL;
                while (tmp->next != tmp->next->next) {
                    if (tmp->elem == elem1)
                        n1 = tmp;
                    else if (tmp->elem == elem2)
                        n2 = tmp;
                    tmp = tmp->next;
                }
                if (n1 != NULL && n2 != NULL) {
                    void *e = n1->elem;
                    n1->elem = n2->elem;
                    n2->elem = e;
                    rc = SUCCESS;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_SINGLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                SinglyNode *n1 = NULL;
                SinglyNode *n2 = NULL;
                while (tmp->next != tmp->next->next) {
                    if (tmp->elem == elem1)
                        n1 = tmp;
                    else if (tmp->elem == elem2)
                        n2 = tmp;
                    tmp = tmp->next;
                }
                if (NULL != n1 && NULL != n2) {
                    void *e = n1->elem;
                    n1->elem = n2->elem;
                    n2->elem = e;
                    rc = SUCCESS;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
    if (ERROR == rc) {
        #ifdef ALGOS_DEBUG
            fprintf(stderr, "%s() error: elements not found in linkedlist\n", FUNC);
        #endif
    }
    return rc;
}

void ll_print(const ll_LinkedList *ll, ll_ElemPrint print)
{
    assert(ll);
    assert(print);

    switch (ll->type) {
        case ll_CIRCLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next->next != ll->s_head) {
                    print(tmp->elem);
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_head->next;
                while (tmp->next != tmp->next->next) {
                    print(tmp->elem);
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_SINGLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                SinglyNode *tmp = ll->s_head->next;
                while (tmp->next != tmp->next->next) {
                    print(tmp->elem);
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
}

void ll_print_reverse(const ll_LinkedList *ll, ll_ElemPrint print)
{
    assert(ll);
    assert(print);

    switch (ll->type) {
        case ll_CIRCLY:
        case ll_SINGLY:
            #ifdef SYNC
                ll_LOCK(&ll->mutex);
            #endif
            _print_reverse(ll->s_head->next, ll->s_tail, print);
            #ifdef SYNC
                ll_UNLOCK(&ll->mutex);
            #endif
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                DoublyNode *tmp = ll->d_tail->prev;
                while (tmp->prev != ll->d_head) {
                    print(tmp->elem);
                    tmp = tmp->prev;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
}

static bool _search_reverse(SinglyNode *start, SinglyNode *end, const void *elem)
{
    if (start == end)
        return false;

    #ifdef SYNC
        ll_LOCK(&ll->mutex);
    #endif
    _search_reverse(start->next, end, elem);
    #ifdef SYNC
        ll_UNLOCK(&ll->mutex);
    #endif

    if (start->elem == elem)
        return true;
    return false;
}

static void _merge(ll_LinkedList *ll, size_t min, size_t mid, size_t max, ll_ElemCompare comp)
{
    void *copy_list[max];
    memset(copy_list, 0, max*sizeof(size_t));

    assert(ll);

    switch (ll->type) {
        case ll_CIRCLY:
        case ll_SINGLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                size_t i, j, k;
                SinglyNode *tmp = ll->s_head;
                SinglyNode *n1 = ll->s_head->next;
                SinglyNode *n2 = NULL;

                for (i = 0; i < min; ++i)
                    n1 = n1->next;
                
                for (j = i, n2 = n1; j < mid; ++j)
                    n2 = n2->next;
               
                for (k = 0; i < mid && j < max; k++) {
                    int result = comp(n1->elem, n2->elem);
                    if (result < 1) {
                        copy_list[k] = n1->elem;
                        n1 = n1->next;
                        i++;
                    } else {
                        copy_list[k] = n2->elem;
                        n2 = n2->next;
                        j++;
                    }
                }

                while (i < mid) {
                    copy_list[k++] = n1->elem;
                    n1 = n1->next;
                    i++;
                }

                while (j < max) {
                    copy_list[k++] = n2->elem;
                    n2 = n2->next;
                    j++;
                }
                
                for (k = 0; k < max; k++) {
                    tmp->next->elem = copy_list[k];
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        case ll_DOUBLY:
            {
                #ifdef SYNC
                    ll_LOCK(&ll->mutex);
                #endif

                size_t i, j, k;
                DoublyNode *tmp = ll->d_head;
                DoublyNode *n1 = ll->d_head->next;
                DoublyNode *n2 = NULL;

                for (i = 0; i < min; ++i)
                    n1 = n1->next;
                
                for (j = i, n2 = n1; j < mid; ++j)
                    n2 = n2->next;
               
                for (k = 0; i < mid && j < max; k++) {
                    int result = comp(n1->elem, n2->elem);
                    if (result < 1) {
                        copy_list[k] = n1->elem;
                        n1 = n1->next;
                        i++;
                    } else {
                        copy_list[k] = n2->elem;
                        n2 = n2->next;
                        j++;
                    }
                }

                while (i < mid) {
                    copy_list[k++] = n1->elem;
                    n1 = n1->next;
                    i++;
                }

                while (j < max) {
                    copy_list[k++] = n2->elem;
                    n2 = n2->next;
                    j++;
                }
                
                for (k = 0; k < max; k++) {
                    tmp->next->elem = copy_list[k];
                    tmp = tmp->next;
                }

                #ifdef SYNC
                    ll_UNLOCK(&ll->mutex);
                #endif
            }
            break;
        default:
            #ifdef ALGOS_DEBUG
                fprintf(stderr, "%s() error: invalid linkedlist type\n", FUNC);
            #endif
            break;
    }
}

static void _print_reverse(SinglyNode *start, SinglyNode *end, ll_ElemPrint print)
{
    if (start == end)
        return;
    _print_reverse(start->next, end, print);
    print(start->elem);
}

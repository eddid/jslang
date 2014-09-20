/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#ifndef _ejstypes_h_
#define _ejstypes_h_

#if defined(__clang__)
#include <stdbool.h>
#endif
#include <stdint.h>
#include "macros.h"
/*
typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef signed long long int64_t;
typedef unsigned long long uint64_t;
*/

typedef int32_t  jsint;
typedef uint32_t jsuint;
typedef double   EJSPrimNumber;
#if defined(JSLANG_UTF8)
typedef char jschar;
#else
typedef uint16_t jschar;
#endif
typedef uint32_t GCObjectHeader;
typedef bool  EJSBool;

#define EJS_TRUE true
#define EJS_FALSE false

#ifndef MIN
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#if IOS
#import <Foundation/Foundation.h>
#define LOG(...) NSLog (@__VA_ARGS__)
#else
#define LOG(...) fprintf (stderr, __VA_ARGS__)
#endif

#define EJS_NOT_IMPLEMENTED() EJS_MACRO_START                           \
    _ejs_log ("%s:%s:%d not implemented.\n", __FILE__, __FUNCTION__, __LINE__); \
    abort();                                                            \
    EJS_MACRO_END

#define EJS_NOT_REACHED() EJS_MACRO_START                               \
    _ejs_log ("%s:%s:%d should not be reached.\n", __FILE__, __FUNCTION__, __LINE__); \
    abort();                                                            \
    EJS_MACRO_END

#define EJS_ASSERT_VAL(assertion,msg,v) ({ EJS_ASSERT_MSG(assertion,msg); (v); })

#define EJS_ASSERT(assertion) EJS_ASSERT_MSG(assertion,#assertion)

#define EJS_ASSERT_MSG(assertion,msg) EJS_MACRO_START                   \
    if (!(assertion)) {                                                 \
        _ejs_log ("%s:%s:%d assertion failed `%s'.\n", __FILE__, __FUNCTION__, __LINE__, (msg)); \
        abort();                                                        \
    }                                                                   \
    EJS_MACRO_END

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __cplusplus
#define EJS_BEGIN_DECLS extern "C" {
#define EJS_END_DECLS }
#else
#define EJS_BEGIN_DECLS
#define EJS_END_DECLS
#endif

#define EJS_MACRO_START do {
#define EJS_MACRO_END } while (0)

#define EJS_STRINGIFY(x) #x

#if defined(__GNUC__) && (__GNUC__ > 2)
# define EJS_LIKELY(x)   (__builtin_expect((x), 1))
# define EJS_UNLIKELY(x) (__builtin_expect((x), 0))
#else
# define EJS_LIKELY(x)   (x)
# define EJS_UNLIKELY(x) (x)
#endif

#define EJS_LIST_HEADER(t)                      \
    t* prev;                                    \
    t* next
#define EJS_LIST_INIT(v) EJS_MACRO_START        \
    (v)->prev = (v)->next = NULL;               \
    EJS_MACRO_END

#define EJS_LIST_PREPEND(v,l) EJS_MACRO_START	\
    v->next = l;                                \
    if (l) l->prev = v;                         \
    l = v;                                      \
    EJS_MACRO_END

#define EJS_LIST_APPEND(t,v,l) EJS_MACRO_START      \
    if ((l) == NULL) {                              \
        EJS_LIST_PREPEND(v,l);                      \
    }                                               \
    else {                                          \
        t* end = l;                                 \
        while (end->next != NULL) end = end->next;  \
        end->next = v;                              \
        v->prev = end;                              \
        v->next = NULL;                             \
    }                                               \
    EJS_MACRO_END

#define EJS_LIST_INSERT_BEFORE(v, l, bef) EJS_MACRO_START   \
    EJS_ASSERT (l);                                         \
    if ((bef) == NULL) {                                    \
        EJS_LIST_PREPEND(v, l);                             \
    }                                                       \
    else {                                                  \
        if ((bef)->prev) {                                  \
            (v)->prev = (bef)->prev;                        \
            (bef)->prev->next = (v);                        \
        }                                                   \
        (v)->next = (bef);                                  \
        (bef)->prev = (v);                                  \
    }                                                       \
    EJS_MACRO_END

#define EJS_LIST_DETACH(v,l) EJS_MACRO_START	\
    if (v->next) v->next->prev = v->prev;		\
    if (v->prev) v->prev->next = v->next;		\
    if (l == v) l = v->next;                    \
    EJS_LIST_INIT(v);                           \
    EJS_MACRO_END


#define EJS_SLIST_HEADER(t)                     \
    t* next
#define EJS_SLIST_INIT(v) EJS_MACRO_START       \
    v->next = NULL;                             \
    EJS_MACRO_END

#define EJS_SLIST_ATTACH(v,l) EJS_MACRO_START	\
    v->next = l;                                \
    l = v;                                      \
    EJS_MACRO_END

#define EJS_SLIST_DETACH_HEAD(v,l) EJS_MACRO_START	\
    l = l->next;                                    \
    EJS_MACRO_END

EJS_BEGIN_DECLS

void jsextern_print_tick();
void jsextern_os_msleep(int mSecond);
bool jsextern_os_swap(int64_t* ptr, int64_t comparand, int64_t replacement);
void *jsextern_pcre_compile(const char *pattern);
void *jsextern_pcre_study(void *compiled_pattern);
int jsextern_pcre_bracketcount(void *compiled_pattern);
int jsextern_pcre_exec(void *compiled_pattern, void *study_result, const char *subject, int length, int start_offset, int *vector, int vector_length);
void jsextern_pcre_free(void *compiled_pattern);
int jsextern_thread_create(void *(*start)(void *), void *arg);
void  jsextern_thread_destroy(int handle);
int jsextern_mutex_create();
void jsextern_mutex_destroy(int handle);
void jsextern_mutex_lock(int handle);
void jsextern_mutex_unlock(int handle);
int jsextern_signal_create();
void jsextern_signal_destroy(int handle);
void jsextern_signal_wait(int handle);
void jsextern_signal_send(int handle);

extern jschar* EJSChar_strdup (const jschar *str);
extern int32_t EJSChar_strcmp (const jschar *s1, const jschar *s2);
#if defined(JSLANG_UTF8)
#define EJSChar_strlen(s) strlen(s)
#else
#define EJSChar_strlen(s) ucs2_strlen(s)
#endif
extern jschar* EJSChar_strstr (const jschar *haystack, const jschar *needle);
extern char* ucs2_to_utf8 (const uint16_t *str);
extern int ucs2_to_utf8_buf(const uint16_t *str, int str_len, char* buf, int buf_size);
extern uint32_t EJSChar_hash (const jschar *str, int hash, int length);

typedef int EJSCompareFunc (void* p1, void* p2);

typedef struct _EJSListNode {
    struct _EJSListNode *prev;
    struct _EJSListNode *next;
} EJSListNode;

typedef struct EJSList {
    EJSListNode* head; // node at the start of the list (head->prev == NULL)
    EJSListNode* tail; // node at the end of the list (tail->next == NULL)
} EJSList;

void _ejs_list_append_node (EJSList *list, EJSListNode* node);
void _ejs_list_prepend_node (EJSList *list, EJSListNode* node);
void _ejs_list_detach_node (EJSList *list, EJSListNode* node);
void _ejs_list_insert_node_sorted (EJSList *list, EJSListNode* node, EJSCompareFunc compare);
void _ejs_list_pop_head (EJSList *list);

int _ejs_list_length (EJSList *list);

#define EJS_LIST_FOREACH(l,t,v,b) for (t* (v) = (t*)(l)->head; (v); (v) = (t*)((EJSListNode*)(v))->next) b

EJS_END_DECLS

#endif /* _ejs_types_h */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#ifndef _ejs_promise_h_
#define _ejs_promise_h_

#include <time.h>
#if !defined(WIN32)
#include <sys/time.h>
#endif
#include "ejs-object.h"

#define EJSVAL_IS_PROMISE(v)     (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_Promise_specops))
#define EJSVAL_TO_PROMISE(v)     ((EJSPromise*)EJSVAL_TO_OBJECT(v))

typedef enum {
    PROMISE_STATE_UNINITIALIZED,
    PROMISE_STATE_PENDING,
    PROMISE_STATE_FULFILLED,
    PROMISE_STATE_REJECTED
} PromiseState;

#define EJS_CAPABILITY_PROMISE_SLOT 0
#define EJS_CAPABILITY_REJECT_SLOT  1
#define EJS_CAPABILITY_RESOLVE_SLOT 2
#define EJS_CAPABILITY_SLOT_COUNT   3

#define EJS_CAPABILITY_NEW() (_ejs_closureenv_new(EJS_CAPABILITY_SLOT_COUNT))
#define EJS_CAPABILITY_GET_PROMISE(cap) (_ejs_closureenv_get_slot(cap, EJS_CAPABILITY_PROMISE_SLOT))
#define EJS_CAPABILITY_GET_REJECT(cap)  (_ejs_closureenv_get_slot(cap, EJS_CAPABILITY_REJECT_SLOT))
#define EJS_CAPABILITY_GET_RESOLVE(cap) (_ejs_closureenv_get_slot(cap, EJS_CAPABILITY_RESOLVE_SLOT))

#define EJS_CAPABILITY_SET_PROMISE(cap,v) (*_ejs_closureenv_get_slot_ref(cap, EJS_CAPABILITY_PROMISE_SLOT) = (v))
#define EJS_CAPABILITY_SET_REJECT(cap,v)  (*_ejs_closureenv_get_slot_ref(cap, EJS_CAPABILITY_REJECT_SLOT)  = (v))
#define EJS_CAPABILITY_SET_RESOLVE(cap,v) (*_ejs_closureenv_get_slot_ref(cap, EJS_CAPABILITY_RESOLVE_SLOT) = (v))

#define EJS_RESOLVEELEMENT_ALREADY_CALLED_SLOT 0
#define EJS_RESOLVEELEMENT_INDEX_SLOT 1
#define EJS_RESOLVEELEMENT_VALUES_SLOT 2
#define EJS_RESOLVEELEMENT_CAPABILITIES_SLOT 3
#define EJS_RESOLVEELEMENT_REMAINING_ELEMENTS_SLOT 4
#define EJS_RESOLVEELEMENT_VALUE_SLOT 5
#define EJS_RESOLVEELEMENT_SLOT_COUNT 6

#define EJS_RESOLVEELEMENT_NEW() (_ejs_closureenv_new(EJS_RESOLVEELEMENT_SLOT_COUNT))
#define EJS_RESOLVEELEMENT_GET_ALREADY_CALLED(re)     (_ejs_closureenv_get_slot(re, EJS_RESOLVEELEMENT_ALREADY_CALLED_SLOT))
#define EJS_RESOLVEELEMENT_GET_INDEX(re)              (_ejs_closureenv_get_slot(re, EJS_RESOLVEELEMENT_INDEX_SLOT))
#define EJS_RESOLVEELEMENT_GET_VALUES(re)             (_ejs_closureenv_get_slot(re, EJS_RESOLVEELEMENT_VALUES_SLOT))
#define EJS_RESOLVEELEMENT_GET_CAPABILITIES(re)       (_ejs_closureenv_get_slot(re, EJS_RESOLVEELEMENT_CAPABILITIES_SLOT))
#define EJS_RESOLVEELEMENT_GET_REMAINING_ELEMENTS(re) (_ejs_closureenv_get_slot(re, EJS_RESOLVEELEMENT_REMAINING_ELEMENTS_SLOT))
#define EJS_RESOLVEELEMENT_GET_VALUE(re)              (_ejs_closureenv_get_slot(re, EJS_RESOLVEELEMENT_VALUE_SLOT))

#define EJS_RESOLVEELEMENT_SET_ALREADY_CALLED(re, v)     (*_ejs_closureenv_get_slot_ref(re, EJS_RESOLVEELEMENT_ALREADY_CALLED_SLOT) = v)
#define EJS_RESOLVEELEMENT_SET_INDEX(re, v)              (*_ejs_closureenv_get_slot_ref(re, EJS_RESOLVEELEMENT_INDEX_SLOT) = v)
#define EJS_RESOLVEELEMENT_SET_VALUES(re, v)             (*_ejs_closureenv_get_slot_ref(re, EJS_RESOLVEELEMENT_VALUES_SLOT) = v)
#define EJS_RESOLVEELEMENT_SET_CAPABILITIES(re, v)       (*_ejs_closureenv_get_slot_ref(re, EJS_RESOLVEELEMENT_CAPABILITIES_SLOT) = v)
#define EJS_RESOLVEELEMENT_SET_REMAINING_ELEMENTS(re, v) (*_ejs_closureenv_get_slot_ref(re, EJS_RESOLVEELEMENT_REMAINING_ELEMENTS_SLOT) = v)
#define EJS_RESOLVEELEMENT_SET_VALUE(re, v)              (*_ejs_closureenv_get_slot_ref(re, EJS_RESOLVEELEMENT_VALUE_SLOT) = v)


typedef struct EJSPromiseReaction {
    EJS_LIST_HEADER(struct EJSPromiseReaction);
    ejsval capabilities;
    ejsval handler;
} EJSPromiseReaction;

typedef struct {
    EJSFunction function;
    ejsval promise;
    ejsval alreadyResolved;
} EJSPromiseResolveFunction;

typedef struct {
    /* object header */
    EJSObject obj;

    /* promise specific data */
    PromiseState state;
    ejsval result;
    ejsval constructor;
    EJSPromiseReaction* fulfillReactions;
    EJSPromiseReaction* rejectReactions;
} EJSPromise;


EJS_BEGIN_DECLS;

extern ejsval _ejs_Promise;
extern ejsval _ejs_Promise_prototype;
extern EJSSpecOps _ejs_Promise_specops;

void _ejs_promise_init(ejsval global);

EJS_END_DECLS;

#endif /* _ejs_promise_h */

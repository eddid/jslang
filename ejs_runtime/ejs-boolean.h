/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#ifndef _ejs_boolean_h_
#define _ejs_boolean_h_

#include "ejs-object.h"

typedef struct EJSBoolean {
    /* object header */
    EJSObject obj;

    /* boolean specific data */
    ejsval boolean_data;
} EJSBoolean;

#define EJSVAL_TO_BOOLEAN_OBJECT(v) ((EJSBoolean*)EJSVAL_TO_OBJECT(v))

EJS_BEGIN_DECLS;

extern ejsval _ejs_Boolean;
extern ejsval _ejs_Boolean_prototype;
extern EJSSpecOps _ejs_Boolean_specops;

void _ejs_boolean_init(ejsval global);

EJS_END_DECLS;

#endif /* _ejs_boolean_h */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#ifndef _ejs_require_h
#define _ejs_require_h

#include "ejs-value.h"
#include "ejs-function.h"

typedef struct {
    const char* name;
    EJSClosureFunc func;
    ejsval cached_exports EJSVAL_ALIGNMENT;
} EJSRequire;

typedef ejsval (*ExternalModuleEntry) (ejsval exports);

typedef struct {
    const char* name;
    ExternalModuleEntry func;
    ejsval cached_exports EJSVAL_ALIGNMENT;
} EJSExternalModuleRequire;

EJS_BEGIN_DECLS;

extern ejsval _ejs_require;

extern void _ejs_require_init(ejsval global);

extern ejsval _ejs_module_get (ejsval name);
extern void   _ejs_module_import_batch (ejsval fromImport, ejsval specifiers, ejsval toExport);

EJS_END_DECLS;

#endif /* _ejs_require_h */

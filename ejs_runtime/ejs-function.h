/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#ifndef _ejs_function_h_
#define _ejs_function_h_

#include "ejs-value.h"
#include "ejs-object.h"
#include "ejs-closureenv.h"

typedef ejsval (*EJSClosureFunc) (ejsval env, ejsval _this, uint32_t argc, ejsval* args);

typedef struct EJSFunction {
    /* object header */
    EJSObject obj;

    EJSClosureFunc func;
    ejsval   env;

    EJSBool  bound;
#if not_anymore
    ejsval   bound_this;
    uint32_t bound_argc;
    ejsval*  bound_args;
#endif
} EJSFunction;


EJS_BEGIN_DECLS;

#define EJSVAL_IS_BOUND_FUNCTION(o) (EJSVAL_IS_FUNCTION(o) && ((EJSFunction*)EJSVAL_TO_OBJECT(o))->bound)

#define EJS_INSTALL_ATOM_FUNCTION(o,n,f) EJS_MACRO_START                \
    ejsval tmpfunc = _ejs_function_new_without_proto (_ejs_null, _ejs_atom_##n, (EJSClosureFunc)f); \
    _ejs_object_setprop (o, _ejs_atom_##n, tmpfunc);                    \
    EJS_MACRO_END

#define EJS_INSTALL_ATOM_FUNCTION_VAL(o,n,f,v) EJS_MACRO_START                    \
    ejsval tmpfunc = _ejs_function_new_without_proto (_ejs_null, _ejs_atom_##n, (EJSClosureFunc)f); \
    _ejs_object_setprop (o, _ejs_atom_##n, tmpfunc);            \
    v = tmpfunc;  \
    EJS_MACRO_END

#define EJS_INSTALL_ATOM_FUNCTION_FLAGS(o,n,f,flags) EJS_MACRO_START         \
    _ejs_object_define_value_property (o, _ejs_atom_##n, _ejs_function_new_without_proto (_ejs_null, _ejs_atom_##n, (EJSClosureFunc)f), flags); \
    EJS_MACRO_END

#define EJS_INSTALL_FUNCTION(o,n,f) EJS_MACRO_START                    \
    ejsval funcname = _ejs_string_new_utf8(n);                          \
    ejsval tmpfunc = _ejs_function_new_without_proto (_ejs_null, funcname, (EJSClosureFunc)f); \
    _ejs_object_setprop (o, funcname, tmpfunc);                         \
    EJS_MACRO_END

#define EJS_INSTALL_FUNCTION_ENV(o,n,f,env) EJS_MACRO_START                \
    ejsval funcname = _ejs_string_new_utf8(n);                          \
    ejsval tmpfunc = _ejs_function_new_without_proto (env, funcname, (EJSClosureFunc)f); \
    _ejs_object_setprop (o, funcname, tmpfunc);                         \
    EJS_MACRO_END

#define EJS_INSTALL_FUNCTION_FLAGS(o,n,f,flags) EJS_MACRO_START         \
    ejsval funcname = _ejs_string_new_utf8(n);                          \
    ejsval tmpfunc = _ejs_function_new_without_proto (_ejs_null, funcname, (EJSClosureFunc)f); \
    _ejs_object_define_value_property (o, funcname, tmpfunc, flags);    \
EJS_MACRO_END

#define EJS_INSTALL_SYMBOL_FUNCTION_FLAGS(o,n,f,flags) EJS_MACRO_START         \
    ejsval tmpfunc = _ejs_function_new_without_proto (_ejs_null, _ejs_atom_##n, (EJSClosureFunc)f); \
    _ejs_object_define_value_property (o, _ejs_Symbol_##n, tmpfunc, flags);    \
EJS_MACRO_END

#define EJS_INSTALL_GETTER(o,n,f) EJS_MACRO_START                     \
    ejsval key = _ejs_string_new_utf8(n);                               \
    ejsval tmpfunc = _ejs_function_new_without_proto (_ejs_null, key, (EJSClosureFunc)f); \
    _ejs_object_define_getter_property(o, key, tmpfunc, EJS_PROP_FLAGS_GETTER_SET); \
    EJS_MACRO_END

#define EJS_INSTALL_ATOM_GETTER(o,n,f) EJS_MACRO_START                     \
    ejsval tmpfunc = _ejs_function_new_without_proto (_ejs_null, _ejs_atom_##n, (EJSClosureFunc)f); \
    _ejs_object_define_getter_property(o, _ejs_atom_##n, tmpfunc, EJS_PROP_FLAGS_GETTER_SET); \
    EJS_MACRO_END

#define EJS_INSTALL_SYMBOL_GETTER(o,n,f) EJS_MACRO_START                     \
    ejsval tmpfunc = _ejs_function_new_without_proto (_ejs_null, _ejs_atom_##n, (EJSClosureFunc)f); \
    _ejs_object_define_getter_property(o, _ejs_Symbol_##n, tmpfunc, EJS_PROP_FLAGS_GETTER_SET); \
    EJS_MACRO_END

#define EJS_INSTALL_ATOM_SETTER(o,n,s) EJS_MACRO_START                  \
    ejsval tmpfunc = _ejs_function_new_without_proto (_ejs_null, _ejs_atom_##n, (EJSClosureFunc)s); \
    _ejs_object_define_setter_property (o, _ejs_atom_##n, tmpfunc, EJS_PROP_FLAGS_SETTER_SET); \
    EJS_MACRO_END

#define EJS_INSTALL_SYMBOL_SETTER(o,n,s) EJS_MACRO_START                  \
    ejsval tmpfunc = _ejs_function_new_without_proto (_ejs_null, _ejs_atom_##n, (EJSClosureFunc)s); \
    _ejs_object_define_setter_property (o, _ejs_Symbol_##n, tmpfunc, EJS_PROP_FLAGS_SETTER_SET); \
    EJS_MACRO_END

ejsval  _ejs_invoke_closure (ejsval closure, ejsval _this, uint32_t argc, ejsval* args);
EJSBool _ejs_invoke_closure_catch (ejsval* retval, ejsval closure, ejsval _this, uint32_t argc, ejsval* args);
EJSBool _ejs_decompose_closure (ejsval closure, EJSClosureFunc* func, ejsval* env, ejsval *_this);

extern ejsval _ejs_function_new (ejsval env, ejsval name, EJSClosureFunc func);
extern ejsval _ejs_function_new_without_proto (ejsval env, ejsval name, EJSClosureFunc func);
extern ejsval _ejs_function_new_anon (ejsval env, EJSClosureFunc func);
extern ejsval _ejs_function_new_utf8 (ejsval env, const char* name, EJSClosureFunc func);

extern ejsval _ejs_function_new_utf8_with_proto (ejsval env, const char* name, EJSClosureFunc func, ejsval prototype);

// used during initialization so we don't create a prototype only to throw it away again
extern ejsval _ejs_function_new_without_proto (ejsval env, ejsval name, EJSClosureFunc func);

extern ejsval _ejs_Function;
extern ejsval _ejs_Function_prototype;
extern EJSSpecOps _ejs_Function_specops;

extern void _ejs_function_init(ejsval global);
extern void _ejs_function_add_symbols();

// used as the __proto__ for a number of builtin objects
ejsval _ejs_Function_empty (ejsval env, ejsval _this, uint32_t argc, ejsval *args);

EJS_END_DECLS;

#endif

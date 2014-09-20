/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#include <math.h>

#include "ejs-value.h"
#include "ejs-gc.h"
#include "ejs-ops.h"
#include "ejs-arguments.h"
#include "ejs-array.h"
#include "ejs-console.h"
#include "ejs-date.h"
#include "ejs-error.h"
#include "ejs-exception.h"
#include "ejs-function.h"
#include "ejs-json.h"
#include "ejs-math.h"
#include "ejs-number.h"
#include "ejs-boolean.h"
#include "ejs-object.h"
#include "ejs-process.h"
#include "ejs-promise.h"
#include "ejs-regexp.h"
#include "ejs-require.h"
#include "ejs-string.h"
#include "ejs-symbol.h"
#include "ejs-typedarrays.h"
#include "ejs-uri.h"
#include "ejs-value.h"
#if IOS || OSX
#include "ejs-xhr.h"
#endif
#include "ejs-map.h"
#include "ejs-set.h"
#if IOS
#include "ejs-webgl.h"
#endif
#include "ejs-proxy.h"
#include "ejs-reflect.h"

ejsval _ejs__ejs EJSVAL_ALIGNMENT;
ejsval _ejs_global EJSVAL_ALIGNMENT;

/* useful strings literals */
#include "ejs-atoms-gen.c"

ejsval
_ejs_eval (ejsval env, ejsval _this, uint32_t argc, ejsval* args)
{
  _ejs_throw_nativeerror_utf8 (EJS_ERROR, "EJS doesn't support eval()");
  return _ejs_undefined;
}

static void
_ejs_init_classes()
{
    _ejs_Class_initialize (&_ejs_Arguments_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_Array_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_ArrayIterator_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_Boolean_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_Date_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_Error_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_Function_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_Map_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_MapIterator_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_Proxy_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_Promise_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_Set_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_SetIterator_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_Number_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_RegExp_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_String_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_StringIterator_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_Symbol_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_ArrayBuffer_specops, &_ejs_Object_specops);

    _ejs_Class_initialize (&_ejs_int8array_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_uint8array_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_int16array_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_uint16array_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_int32array_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_uint32array_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_float32array_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_float64array_specops, &_ejs_Object_specops);

    _ejs_Class_initialize (&_ejs_DataView_specops, &_ejs_Object_specops);
#if IOS
    _ejs_Class_initialize (&_ejs_WebGLRenderingContext_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_WebGLBuffer_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_WebGLFramebuffer_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_WebGLRenderbuffer_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_WebGLProgram_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_WebGLShader_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_WebGLTexture_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_WebGLActiveInfo_specops, &_ejs_Object_specops);
    _ejs_Class_initialize (&_ejs_WebGLUniformLocation_specops, &_ejs_Object_specops);
#endif
#if IOS || OSX
    _ejs_Class_initialize (&_ejs_XMLHttpRequest_specops, &_ejs_Object_specops);
#endif
}

void
_ejs_init(int argc, char** argv)
{
    // process class inheritance
    _ejs_init_classes();

    // initialize our atoms before anything else
    _ejs_init_static_strings();

    _ejs_gc_init();
    _ejs_exception_init();

    // initialization or ECMA262 builtins
    _ejs_gc_add_root (&_ejs_global);
    _ejs_global = _ejs_object_new (_ejs_null, &_ejs_Object_specops);

    _ejs_object_setprop (_ejs_global, _ejs_atom_undefined, _ejs_undefined);
    _ejs_object_setprop (_ejs_global, _ejs_atom_NaN, _ejs_nan);
    _ejs_object_setprop (_ejs_global, _ejs_atom_Infinity, jsPositiveInfinity);
    _ejs_object_setprop (_ejs_global, _ejs_atom_eval, _ejs_function_new_without_proto (_ejs_undefined, _ejs_atom_eval, _ejs_eval));

    _ejs_object_init_proto();

    _ejs_function_init(_ejs_global);

    _ejs_object_init(_ejs_global);

    _ejs_symbol_init(_ejs_global);

    _ejs_function_add_symbols();

    _ejs_reflect_init(_ejs_global);
    _ejs_error_init(_ejs_global);
    _ejs_arguments_init(_ejs_global);
    _ejs_array_init(_ejs_global);
    _ejs_boolean_init (_ejs_global);
    _ejs_string_init(_ejs_global);
    _ejs_number_init(_ejs_global);
    _ejs_regexp_init(_ejs_global);
    _ejs_date_init(_ejs_global);
    _ejs_json_init(_ejs_global);
    _ejs_math_init(_ejs_global);

    // ES6 bits
    _ejs_promise_init(_ejs_global);
    _ejs_proxy_init(_ejs_global);
    _ejs_map_init(_ejs_global);
    _ejs_set_init(_ejs_global);

    _ejs_typedarrays_init(_ejs_global);
#if IOS
    _ejs_webgl_init(_ejs_global);
#endif

#define GLOBAL_METHOD(x) EJS_MACRO_START                                \
    _ejs_##x = _ejs_function_new_without_proto (_ejs_null, _ejs_atom_##x, (EJSClosureFunc)_ejs_##x##_impl); \
    _ejs_object_setprop (_ejs_global, _ejs_atom_##x, _ejs_##x);         \
    EJS_MACRO_END

    GLOBAL_METHOD(isNaN);
    GLOBAL_METHOD(isFinite);
    GLOBAL_METHOD(parseInt);
    GLOBAL_METHOD(parseFloat);

    GLOBAL_METHOD(decodeURI);
    GLOBAL_METHOD(decodeURIComponent);
    GLOBAL_METHOD(encodeURI);
    GLOBAL_METHOD(encodeURIComponent);

#undef GLOBAL_METHOD

    // the node-like api we support in order for our driver to
    // function.  this should really be a separate opt-in .a/.so.
    _ejs_require_init(_ejs_global);
    _ejs_console_init(_ejs_global);
    _ejs_process_init(_ejs_global, argc, argv);

#if IOS || OSX
    _ejs_xmlhttprequest_init(_ejs_global);
#endif

    // a special global (__ejs) under which we can stuff other
    // semi-useful runtime features, like a call to force a GC.  the
    // compiler also uses the presence of __ejs to disable
    // buggy/nonfunctional code (like those that use regexps)
    _ejs__ejs = _ejs_object_new (_ejs_null, &_ejs_Object_specops);
    _ejs_object_setprop (_ejs_global, _ejs_atom___ejs, _ejs__ejs);

    _ejs_GC_init(_ejs__ejs);
    _ejs_gc_allocate_oom_exceptions();
}

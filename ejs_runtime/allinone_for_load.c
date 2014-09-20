/**
 * eddid:
 * since all eji-runtime in one file is too big for LLVM to parse
 * so I split it in two file:
 * allinone_for_link.c, include all usefull APIs, global variables, structs, and so on
 * allinone_for_load.c, just include the structs and APIs declared for AST
 */

#include <stdio.h>
#include <stdlib.h>
#include "ejs-ops.h"
#include "ejs-arguments.h"
#include "ejs-array.h"
#include "ejs-boolean.h"
#include "ejs-console.h"
#include "ejs-function.h"
#include "ejs-json.h"
#include "ejs-math.h"
#include "ejs-number.h"
#include "ejs-process.h"
#include "ejs-regexp.h"
#include "ejs-string.h"
#include "ejs-symbol.h"

int IsLitterEndian()
{
    union ejsval u;
    u.asInt64 = 1;
    if (1 == u.asIntPtr[0])
    {
        printf("litterendian\n");
        jsValue32PayloadPart = 0;
        jsValue32TagPart = 1;
        isLitterEndian = 1;
        return 1;
    }
    printf("bigendian\n");
    return 0;
}

void AllInOneInit()
{
    IsLitterEndian();
    
    _ejs_nan.asInt64 = 0x7FF8000000000000LL;
    jsPositiveInfinity.asInt64 = 0x7FF0000000000000LL;
    jsNegativeInfinity.asInt64 = 0xFFF0000000000000LL;
    _ejs_null.asIntPtr[jsValue32TagPart] = EJSVAL_TAG_NULL;
    _ejs_null.asIntPtr[jsValue32PayloadPart] = 0x00000000;
    _ejs_undefined.asIntPtr[jsValue32TagPart] = EJSVAL_TAG_UNDEFINED;
    _ejs_undefined.asIntPtr[jsValue32PayloadPart] = 0x00000000;
    _ejs_true.asIntPtr[jsValue32TagPart] = EJSVAL_TAG_BOOLEAN;
    _ejs_true.asIntPtr[jsValue32PayloadPart] = 0x00000001;
    _ejs_false.asIntPtr[jsValue32TagPart] = EJSVAL_TAG_BOOLEAN;
    _ejs_false.asIntPtr[jsValue32PayloadPart] = 0x00000000;
    _ejs_one.asDouble = 1;
    _ejs_zero.asDouble = 0;
    jsMin.asDouble = 5E-324;
    jsMax.asDouble = 1.7976931348623157E+308;

    _ejs_init(0, 0);
    printf("%s:%d\n", __FUNCTION__, __LINE__);
}

void allinone_for_load_just_ensure_these_functions_and_variables_are_included_please_do_not_call()
{
    JSValueHash(_ejs_nan);
    JSValueHash(jsPositiveInfinity);
    JSValueHash(jsNegativeInfinity);
    JSValueHash(jsMax);
    JSValueHash(jsMin);
    JSValueHash(_ejs_null);
    JSValueHash(_ejs_undefined);
    JSValueHash(_ejs_true);
    JSValueHash(_ejs_false);
    JSValueHash(_ejs_one);
    JSValueHash(_ejs_zero);
    JSValueHash(_ejs_global);
    JSValueHash(_ejs_console);
    JSValueHash(_ejs_Object);
    JSValueHash(_ejs_Boolean);
    JSValueHash(_ejs_Number);
    JSValueHash(_ejs_String);
    JSValueHash(_ejs_Array);
    JSValueHash(_ejs_Function);
    JSValueHash(_ejs_Process);
    JSValueHash(_ejs_Symbol_create);
    JSValueHash(_ejs_Math);
    JSValueHash(_ejs_JSON);

    jsextern_print_tick();
    jsextern_os_msleep(0);
    jsextern_os_swap(NULL, 0, 0);
    jsextern_pcre_compile(NULL);
    jsextern_pcre_study(NULL);
    jsextern_pcre_bracketcount(NULL);
    jsextern_pcre_exec(NULL, NULL, NULL, 0, 0, NULL, 0);
    jsextern_pcre_free(NULL);
    jsextern_thread_create((void *(*)(void *))NULL, NULL);
    jsextern_thread_destroy(0);
    jsextern_mutex_create();
    jsextern_mutex_destroy(0);
    jsextern_mutex_lock(0);
    jsextern_mutex_unlock(0);
    jsextern_signal_create();
    jsextern_signal_destroy(0);
    jsextern_signal_wait(0);
    jsextern_signal_send(0);

    JSValueHash(_ejs_undefined);
    
    _ejs_eval(_ejs_undefined, _ejs_undefined, 0, NULL);

    //Object
    _ejs_object_getprop_utf8(_ejs_undefined, NULL);
    _ejs_object_setprop_utf8(_ejs_global, NULL, _ejs_undefined);
    _ejs_object_define_value_property(_ejs_undefined, _ejs_undefined, _ejs_undefined, 0);
    _ejs_object_define_getter_property(_ejs_undefined, _ejs_undefined, _ejs_undefined, 0);
    _ejs_object_define_setter_property(_ejs_undefined, _ejs_undefined, _ejs_undefined, 0);
    _ejs_Object_create(_ejs_undefined, _ejs_undefined, 0, NULL);
    _ejs_Object_getOwnPropertyNames(_ejs_undefined, _ejs_undefined, 0, NULL);
    //vtable
    _ejs_specop_get(_ejs_undefined, _ejs_undefined, _ejs_undefined);
    _ejs_specop_set(_ejs_undefined, _ejs_undefined, _ejs_undefined, _ejs_undefined);
    //option
    ToEJSBool(_ejs_undefined);
    ToDouble(_ejs_undefined);
    ToUint32(_ejs_undefined);
    _ejs_op_typeof(_ejs_undefined);
    _ejs_op_instanceof(_ejs_undefined, _ejs_undefined);
    _ejs_op_typeof_is_array(_ejs_undefined);
    _ejs_op_plusplus(_ejs_undefined, EJS_FALSE);
    _ejs_op_minusminus(_ejs_undefined, EJS_FALSE);
    _ejs_op_bitwise_xor(_ejs_undefined, _ejs_undefined);
    _ejs_op_bitwise_and(_ejs_undefined, _ejs_undefined);
    _ejs_op_bitwise_or(_ejs_undefined, _ejs_undefined);
    _ejs_op_rsh(_ejs_undefined, _ejs_undefined);
    _ejs_op_ursh(_ejs_undefined, _ejs_undefined);
    _ejs_op_lsh(_ejs_undefined, _ejs_undefined);
    _ejs_op_ulsh(_ejs_undefined, _ejs_undefined);
    _ejs_op_mod(_ejs_undefined, _ejs_undefined);
    _ejs_op_add(_ejs_undefined, _ejs_undefined);
    _ejs_op_sub(_ejs_undefined, _ejs_undefined);
    _ejs_op_mult(_ejs_undefined, _ejs_undefined);
    _ejs_op_div(_ejs_undefined, _ejs_undefined);
    _ejs_op_lt(_ejs_undefined, _ejs_undefined);
    _ejs_op_le(_ejs_undefined, _ejs_undefined);
    _ejs_op_gt(_ejs_undefined, _ejs_undefined);
    _ejs_op_ge(_ejs_undefined, _ejs_undefined);
    _ejs_op_strict_eq(_ejs_undefined, _ejs_undefined);
    _ejs_op_strict_neq(_ejs_undefined, _ejs_undefined);
    _ejs_op_eq(_ejs_undefined, _ejs_undefined);
    _ejs_op_neq(_ejs_undefined, _ejs_undefined);
    //arguments
    _ejs_arguments_new(0, NULL);

    //Array
    _ejs_array_new(jsValue32Size, false);
    //String
    _ejs_string_new_utf8_len(NULL, 0);
    //Function
    _ejs_function_new_utf8(_ejs_undefined, NULL, (EJSClosureFunc)NULL);
    _ejs_invoke_closure(_ejs_undefined, _ejs_undefined, 0, NULL);
    //RegExp
    _ejs_regexp_new_utf8(NULL, NULL);
}

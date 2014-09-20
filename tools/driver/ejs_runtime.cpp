/**
 * jslang, A Javascript AOT compiler base on LLVM
 *
 * Copyright (c) 2014 Eddid Zhang <zhangheng607@163.com>
 * All Rights Reserved.
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 */

#include <direct.h>
#include "allinone_for_load.c"
#include "allinone_for_link.cpp"

void test_object_property()
{
    //var obj1 = { 100: "a", 2: "b", 7: "c"};
    ejsval obj1 = _ejs_Object_create(_ejs_global, _ejs_global, 1, &_ejs_null);
    ejsval key = NUMBER_TO_EJSVAL(100);
    ejsval value = _ejs_string_new_utf8("a");
    _ejs_object_define_value_property(obj1, key, value, EJS_PROP_FLAGS_ENUMERABLE | EJS_PROP_FLAGS_ENUMERABLE_SET);
    ejsval properties = _ejs_Object_getOwnPropertyNames(_ejs_undefined, _ejs_undefined, 1, &obj1);
    ejsval property = _ejs_specop_get(obj1, key, obj1);
    _ejs_dump_value(property);
}

ejsval debug;
ejsval noname;
ejsval test_function_closure_debug(ejsval env, ejsval _this, uint32_t argc, ejsval* args)
{
    return _ejs_zero;
}

ejsval test_function_closure_noname(ejsval env, ejsval _this, uint32_t argc, ejsval* args)
{
    _ejs_invoke_closure(debug, _this, 0, NULL);
    return _ejs_zero;
}

void test_function_closure()
{
    //var debug = function () { console.log("hi"); };
    //var noname = function () { debug(); };
    //noname();

    debug = _ejs_function_new_utf8(_ejs_global, NULL, test_function_closure_debug);
    noname = _ejs_function_new_utf8(_ejs_global, NULL, test_function_closure_noname);
    _ejs_invoke_closure(noname, _ejs_global, 0, NULL);
}

int main(int argn, const char *argv[])
{
    AllInOneInit();
    test_function_closure();
}

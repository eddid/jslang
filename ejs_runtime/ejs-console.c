/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#include <math.h>

#include "ejs-console.h"
#include "ejs-gc.h"
#include "ejs-ops.h"
#include "ejs-function.h"
#include "ejs-string.h"
#include "ejs-error.h"
#include "ejs-array.h"

#if IOS
#import <Foundation/Foundation.h>
#endif

static ejsval
output (FILE *outfile, uint32_t argc, ejsval *args)
{
#if IOS
#define OUTPUT0(str) NSLog(@str)
#define OUTPUT(format, val) NSLog(@format, val)
#else
#define OUTPUT0(str) fprintf (outfile, str)
#define OUTPUT(format, val) fprintf (outfile, format, val)
#endif

    for (uint32_t i = 0; i < argc; i++) {
        if (EJSVAL_IS_NUMBER(args[i])) {
            double d = EJSVAL_TO_NUMBER(args[i]);
            int di;
            if (EJSDOUBLE_IS_INT32(d, &di))
                OUTPUT ("%d", di);
            else {
                int classified = fpclassify(d);
                if (classified == FP_INFINITE) {
                    if (d < 0)
                        OUTPUT0 ("-Infinity");
                    else
                        OUTPUT0 ("Infinity");
                }
                else if (classified == FP_NAN) {
                    OUTPUT0 ("NaN");
                }
                else
                    OUTPUT (EJS_NUMBER_FORMAT, d);
            }
        }
        else if (EJSVAL_IS_ARRAY(args[i])) {
            char* strval_utf8;

            if (EJS_ARRAY_LEN(args[i]) == 0) {
                OUTPUT0 ("[]");
            }
            else {
                ejsval comma_space = _ejs_string_new_utf8(", ");
                ejsval lbracket = _ejs_string_new_utf8("[ ");
                ejsval rbracket = _ejs_string_new_utf8(" ]");

                ejsval contents = _ejs_array_join (args[i], comma_space);

                strval_utf8 = _ejs_string_to_utf8(EJSVAL_TO_STRING(_ejs_string_concatv(lbracket, contents, rbracket, _ejs_null)));

                OUTPUT ("%s", strval_utf8);
                free (strval_utf8);
            }
        }
        else if (EJSVAL_IS_ERROR(args[i])) {
            ejsval strval = ToString(args[i]);

            char* strval_utf8 = _ejs_string_to_utf8(EJSVAL_TO_STRING(strval));
            OUTPUT ("[%s]", strval_utf8);
            free (strval_utf8);
        }
        else if (EJSVAL_IS_FUNCTION(args[i])) {
            ejsval func_name = _ejs_object_getprop (args[i], _ejs_atom_name);

            if (EJSVAL_IS_NULL_OR_UNDEFINED(func_name) || EJSVAL_TO_STRLEN(func_name) == 0) {
                OUTPUT0("[Function]");
            }
            else {
                char* strval_utf8 = _ejs_string_to_utf8(EJSVAL_TO_STRING(func_name));
                OUTPUT ("[Function: %s]", strval_utf8);
                free (strval_utf8);
            }
        }
        else {
            ejsval strval = ToString(args[i]);

            char* strval_utf8 = _ejs_string_to_utf8(EJSVAL_TO_STRING(strval));
            OUTPUT ("%s", strval_utf8);
            free (strval_utf8);
        }
#if !IOS
        if (i < argc - 1)
            fputc (' ', outfile);
#endif
    }

#if !IOS
    fputc ('\n', outfile);
#endif

    return _ejs_undefined;
}

static ejsval
_ejs_console_log (ejsval env, ejsval _this, uint32_t argc, ejsval *args)
{
    return output (stdout, argc, args);
}

static ejsval
_ejs_console_warn (ejsval env, ejsval _this, uint32_t argc, ejsval *args)
{
    return output (stderr, argc, args);
}

ejsval _ejs_console EJSVAL_ALIGNMENT;

void
_ejs_console_init(ejsval global)
{
    _ejs_console = _ejs_object_new (_ejs_null, &_ejs_Object_specops);
    _ejs_object_setprop (global, _ejs_atom_console, _ejs_console);

#define OBJ_METHOD(x) EJS_INSTALL_ATOM_FUNCTION(_ejs_console, x, _ejs_console_##x)

    OBJ_METHOD(log);
    OBJ_METHOD(warn);

#undef OBJ_METHOD
}

#include "ejs-value.c"
#include "ejs-arguments.c"
#include "ejs-array.c"
#include "ejs-boolean.c"
#include "ejs-closureenv.c"
#include "ejs-console.c"
#include "ejs-date.c"
#include "ejs-error.c"
#include "ejs-exception.c"
#include "ejs-function.c"
#include "ejs-gc.c"
#include "ejs-init.c"
#include "ejs-json.c"
#include "ejs-map.c"
#include "ejs-math.c"
#include "ejs-number.c"
#include "ejs-object.c"
#include "ejs-ops.c"
#include "ejs-process.c"
#include "ejs-promise.c"
#include "ejs-proxy.c"
#include "ejs-recording.c"
#include "ejs-reflect.c"
#include "ejs-regexp.c"
#include "ejs-require.c"
#include "ejs-set.c"
#include "ejs-string.c"
#include "ejs-symbol.c"
#include "ejs-typedarrays.c"
#include "ejs-types.c"
#include "ejs-uri.c"
#include "ejs-log.c"
#include "parson.c"

EJSBool _ejs_invoke_closure_catch (ejsval* retval, ejsval closure, ejsval _this, uint32_t argc, ejsval* args)
{
    return EJS_FALSE;
}

int jsValue32Size = sizeof(union ejsval);
int jsValue32PayloadPart = 1;
int jsValue32TagPart = 0;
int isLitterEndian = 0;

union ejsval _ejs_nan;
union ejsval jsPositiveInfinity;
union ejsval jsNegativeInfinity;
union ejsval jsMax;
union ejsval jsMin;
union ejsval _ejs_null;
union ejsval _ejs_undefined;
union ejsval _ejs_true;
union ejsval _ejs_false;
union ejsval _ejs_one;
union ejsval _ejs_zero;

//_ejs_dump_value ?
ejsval echo(ejsval env, ejsval thisObject, int argc, ejsval* argv)
{
    if (argc < 1)
    {
        return _ejs_undefined;
    }
    ejsval jsValue = argv[0];
    ejsval jsStr = ToString(jsValue);
    char *utf8Str = _ejs_string_to_utf8(EJSVAL_TO_STRING(jsStr));
    printf("%s\n", utf8Str);
    free(utf8Str);

    return _ejs_undefined;
}

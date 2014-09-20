/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#ifndef _ejs_value_h
#define _ejs_value_h
/*
 * Implementation details for js::Value in jsapi.h.
 */
#include "ejs-types.h"

EJS_BEGIN_DECLS;

// notyet for these
#if defined(__GNUC__) || defined(__xlc__) || defined(__xlC__)
#define EJS_ALWAYS_INLINE //__attribute__((always_inline))
#else
#define EJS_ALWAYS_INLINE inline
#endif

#define EJS_STATIC_ASSERT(x)

/******************************************************************************/

/*
 * Try to get jsvals 64-bit aligned. We could almost assert that all values are
 * aligned, but MSVC and GCC occasionally break alignment.
 */
#if defined(__GNUC__) || defined(__xlc__) || defined(__xlC__)
# define EJSVAL_ALIGNMENT        __attribute__((aligned (8)))
#elif defined(_MSC_VER)
  /*
   * Structs can be aligned with MSVC, but not if they are used as parameters,
   * so we just don't try to align.
   */
# define EJSVAL_ALIGNMENT
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
# define EJSVAL_ALIGNMENT
#elif defined(__HP_cc) || defined(__HP_aCC)
# define EJSVAL_ALIGNMENT
#endif

/*
 * We try to use enums so that printing a ejsval in the debugger shows
 * nice symbolic type tags, however we can only do this when we can force the
 * underlying type of the enum to be the desired size.
 */
typedef uint32_t EJSValueTag;

#if EJS_BITS_PER_WORD == 32

#define EJSVAL_TAG_CLEAR              ((uint32_t)(0xFFFFFF80))
#define EJSVAL_TAG_INT32              ((uint32_t)(0xFFFFFF81))
#define EJSVAL_TAG_UNDEFINED          ((uint32_t)(0xFFFFFF82))
#define EJSVAL_TAG_STRING             ((uint32_t)(0xFFFFFF83))
#define EJSVAL_TAG_CLOSUREENV         ((uint32_t)(0xFFFFFF84))
#define EJSVAL_TAG_BOOLEAN            ((uint32_t)(0xFFFFFF85))
#define EJSVAL_TAG_MAGIC              ((uint32_t)(0xFFFFFF86))
#define EJSVAL_TAG_NULL               ((uint32_t)(0xFFFFFF87))
#define EJSVAL_TAG_OBJECT             ((uint32_t)(0xFFFFFF88))

#elif EJS_BITS_PER_WORD == 64

#define EJSVAL_TAG_SHIFT 47
#define EJSVAL_TAG_MAX_DOUBLE         ((uint32_t)0x1FFF0)
#define EJSVAL_TAG_INT32              ((uint32_t)0x1FFF1)
#define EJSVAL_TAG_UNDEFINED          ((uint32_t)0x1FFF2)
#define EJSVAL_TAG_NULL               ((uint32_t)0x1FFF3)
#define EJSVAL_TAG_STRING             ((uint32_t)0x1FFF4)
#define EJSVAL_TAG_CLOSUREENV         ((uint32_t)0x1FFF5)
#define EJSVAL_TAG_BOOLEAN            ((uint32_t)0x1FFF6)
#define EJSVAL_TAG_MAGIC              ((uint32_t)0x1FFF7)
#define EJSVAL_TAG_OBJECT             ((uint32_t)0x1FFF8)

#define EJSVAL_PAYLOAD_MASK           0x00007FFFFFFFFFFFLL
#define EJSVAL_TAG_MASK               0xFFFF800000000000LL

typedef uint64_t EJSValueShiftedTag;
#define EJSVAL_SHIFTED_TAG_MAX_DOUBLE ((uint64_t)0xFFF80000FFFFFFFFLL)
#define EJSVAL_SHIFTED_TAG_INT32      ((uint64_t)0xFFF8100000000000LL)
#define EJSVAL_SHIFTED_TAG_UNDEFINED  ((uint64_t)0xFFF8200000000000LL)
#define EJSVAL_SHIFTED_TAG_STRING     ((uint64_t)0xFFF8300000000000LL)
#define EJSVAL_SHIFTED_TAG_CLOSUREENV ((uint64_t)0xFFF8400000000000LL)
#define EJSVAL_SHIFTED_TAG_BOOLEAN    ((uint64_t)0xFFF8500000000000LL)
#define EJSVAL_SHIFTED_TAG_MAGIC      ((uint64_t)0xFFF8600000000000LL)
#define EJSVAL_SHIFTED_TAG_NULL       ((uint64_t)0xFFF8700000000000LL)
#define EJSVAL_SHIFTED_TAG_OBJECT     ((uint64_t)0xFFF8800000000000LL)

#endif  /* EJS_BITS_PER_WORD */

#define EJS_PROP_FLAGS_ENUMERABLE       ((uint32_t)0x00000001)
#define EJS_PROP_FLAGS_CONFIGURABLE     ((uint32_t)0x00000002)
#define EJS_PROP_FLAGS_WRITABLE         ((uint32_t)0x00000004)
//((uint32_t)0x00000008)
#define EJS_PROP_FLAGS_ENUMERABLE_SET   ((uint32_t)0x00000010)
#define EJS_PROP_FLAGS_CONFIGURABLE_SET ((uint32_t)0x00000020)
#define EJS_PROP_FLAGS_WRITABLE_SET     ((uint32_t)0x00000040)
#define EJS_PROP_FLAGS_VALUE_SET        ((uint32_t)0x00000080)
#define EJS_PROP_FLAGS_GETTER_SET       ((uint32_t)0x00000100)
#define EJS_PROP_FLAGS_SETTER_SET       ((uint32_t)0x00000200)

typedef enum EJSWhyMagic
{
    EJS_ARRAY_HOLE,               /* a hole in a dense array */
    EJS_ARGS_HOLE,                /* a hole in the args object's array */
    EJS_NATIVE_ENUMERATE,         /* indicates that a custom enumerate hook forwarded
                                  * to EJS_EnumerateState, which really means the object can be
                                  * enumerated like a native object. */
    EJS_NO_ITER_VALUE,            /* there is not a pending iterator value */
    EJS_GENERATOR_CLOSING,        /* exception value thrown when closing a generator */
    EJS_NO_CONSTANT,              /* compiler sentinel value */
    EJS_THIS_POISON,              /* used in debug builds to catch tracing errors */
    EJS_ARG_POISON,               /* used in debug builds to catch tracing errors */
    EJS_SERIALIZE_NO_NODE,        /* an empty subnode in the AST serializer */
    EJS_LAZY_ARGUMENTS,           /* lazy arguments value on the stack */
    EJS_IS_CONSTRUCTING,          /* magic value passed to natives to indicate construction */
    EJS_GENERIC_MAGIC             /* for local use */
} EJSWhyMagic;

typedef union ejsval
{
    uint64_t asInt64;
    uint32_t asIntPtr[2];
    double asDouble;
} ejsval;

EJS_STATIC_ASSERT(sizeof(ejsval) == 8);

/******************************************************************************/

/* To avoid a circular dependency, pull in the necessary pieces of jsnum.h. */

#define EJSDOUBLE_SIGNBIT (((uint64_t) 1) << 63)
#define EJSDOUBLE_EXPMASK (((uint64_t) 0x7ff) << 52)
#define EJSDOUBLE_MANTMASK ((((uint64_t) 1) << 52) - 1)
#define EJSDOUBLE_HI32_SIGNBIT   0x80000000

extern int jsValue32Size;
extern int jsValue32PayloadPart;
extern int jsValue32TagPart;
extern int isLitterEndian;

extern ejsval jsMax;
extern ejsval jsMin;
extern ejsval _ejs_undefined;
extern ejsval _ejs_null;
extern ejsval _ejs_nan;
extern ejsval jsNegativeInfinity;
extern ejsval jsPositiveInfinity;
extern ejsval _ejs_true;
extern ejsval _ejs_false;
extern ejsval _ejs_zero;
extern ejsval _ejs_one;
extern ejsval _ejs_global;

#define EJSVAL_IS_PRIMITIVE(v) (EJSVAL_IS_NUMBER(v) || EJSVAL_IS_STRING(v) || EJSVAL_IS_BOOLEAN(v) || EJSVAL_IS_UNDEFINED(v))

#define EJSVAL_IS_OBJECT(v)    EJSVAL_IS_OBJECT_IMPL(v)
#define EJSVAL_IS_ARRAY(v)     (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_Array_specops || EJSVAL_TO_OBJECT(v)->ops == &_ejs_sparsearray_specops))
#define EJSVAL_IS_FUNCTION(v)  (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_Function_specops))
#define EJSVAL_IS_CALLABLE(v)  (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_Function_specops || EJSVAL_TO_OBJECT(v)->ops == &_ejs_Proxy_specops))
#define EJSVAL_IS_CONSTRUCTOR(v)  (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_Function_specops || EJSVAL_TO_OBJECT(v)->ops == &_ejs_Proxy_specops)) // XXX this is lacking
#define EJSVAL_IS_DATE(v)      (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_Date_specops))
#define EJSVAL_IS_REGEXP(v)    (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_RegExp_specops))
#define EJSVAL_IS_NUMBER_OBJECT(v) (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_Number_specops))
#define EJSVAL_IS_STRING_OBJECT(v) (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_String_specops))
#define EJSVAL_IS_BOOLEAN_OBJECT(v) (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_Boolean_specops))
#define EJSVAL_IS_NUMBER(v)    EJSVAL_IS_DOUBLE_IMPL(v)
#define EJSVAL_IS_INT32(v)    EJSVAL_IS_INT32_IMPL(v)
#define EJSVAL_IS_STRING(v)    EJSVAL_IS_STRING_IMPL(v)
#define EJSVAL_IS_SYMBOL(v)    (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_Symbol_specops))
#define EJSVAL_IS_CLOSUREENV(v) EJSVAL_IS_CLOSUREENV_IMPL(v)
#define EJSVAL_IS_BOOLEAN(v)   EJSVAL_IS_BOOLEAN_IMPL(v)
#define EJSVAL_IS_UNDEFINED(v) EJSVAL_IS_UNDEFINED_IMPL(v)
#define EJSVAL_IS_NULL(v)      EJSVAL_IS_NULL_IMPL(v)
#define EJSVAL_IS_OBJECT_OR_NULL(v) EJSVAL_IS_OBJECT_OR_NULL_IMPL(v)
#define EJSVAL_IS_NULL_OR_UNDEFINED(v) (EJSVAL_IS_NULL(v) || EJSVAL_IS_UNDEFINED(v))
#define EJSVAL_IS_NO_ITER_VALUE_MAGIC(v) (EJSVAL_IS_MAGIC_IMPL(v) && EJSVAL_TO_INT32_IMPL(v) == EJS_NO_ITER_VALUE)
#define EJSVAL_IS_ARRAY_HOLE_MAGIC(v) (EJSVAL_IS_MAGIC_IMPL(v) && EJSVAL_TO_INT32_IMPL(v) == EJS_ARRAY_HOLE)

#define EJSOBJECT_IS_ARRAY(v)     ((v)->ops == &_ejs_Array_specops || (v)->ops == &_ejs_sparsearray_specops)

#define EJSVAL_TO_OBJECT(v)       EJSVAL_TO_OBJECT_IMPL(v)
#define EJSVAL_TO_NUMBER(v)       v.asDouble
#define EJSVAL_TO_INT32(v)        EJSVAL_TO_INT32_IMPL(v)
#define EJSVAL_TO_BOOLEAN(v)      EJSVAL_TO_BOOLEAN_IMPL(v)
#define EJSVAL_TO_FUNC(v)         ((EJSFunction*)EJSVAL_TO_OBJECT_IMPL(v))->func
#define EJSVAL_TO_ENV(v)          ((EJSFunction*)EJSVAL_TO_OBJECT_IMPL(v))->env

#define OBJECT_TO_EJSVAL(v)       OBJECT_TO_EJSVAL_IMPL((EJSObject*)v)
#define BOOLEAN_TO_EJSVAL(v)      BOOLEAN_TO_EJSVAL_IMPL(v)
#define NUMBER_TO_EJSVAL(v)       DOUBLE_TO_EJSVAL_IMPL(v)
#define STRING_TO_EJSVAL(v)       STRING_TO_EJSVAL_IMPL(v)

#define EJSVAL_EQ(v1,v2)          ((v1).asInt64 == (v2).asInt64)

#define EJS_NUMBER_FORMAT "%g"

void _ejs_init(int argc, char** argv);

#define EJS_ATOM(atom) extern ejsval _ejs_atom_##atom; extern const jschar _ejs_buffer_##atom[];
#define EJS_ATOM2(atom,atom_name) extern ejsval _ejs_atom_##atom_name; extern const jschar _ejs_buffer_##atom_name[];
#include "ejs-atoms.h"

EJS_END_DECLS;

#endif /* _ejs_value_h */

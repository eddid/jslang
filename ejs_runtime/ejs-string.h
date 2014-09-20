/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */
#ifndef _ejs_string_h_
#define _ejs_string_h_

#include "ejs-object.h"

typedef struct EJSString {
    /* object header */
    EJSObject obj;

    /* string specific data */
    ejsval primStr;
} EJSString;

EJS_BEGIN_DECLS;

extern ejsval _ejs_String;
extern ejsval _ejs_String__proto__;
extern ejsval _ejs_String_prototype;
extern EJSSpecOps _ejs_String_specops;

void _ejs_string_init(ejsval global);



/* primitive strings can exist in a few different forms

   1: a statically allocated C string that isn't freed when the
      primitive string is destroyed.  We use these for atoms - known
      strings in both the runtime and compiler and for strings that
      appear in JS source as literals.

   2: ropes built up by concatenating strings together.

   3: dependent strings made by taking substrings/slices of other
      strings when the resulting string is large and we don't want to
      waste a lot of space with a copy.
*/
#define EJSVAL_TO_FLAT_STRING(v)  _ejs_string_flatten(v)->data.flat
#define EJSVAL_TO_STRING(v)       EJSVAL_TO_STRING_IMPL(v)
#define EJSVAL_TO_STRLEN(v)       EJSVAL_TO_STRING_IMPL(v)->length

typedef enum {
    EJS_STRING_FLAT      = 1,
    EJS_STRING_ROPE      = 2,
    EJS_STRING_DEPENDENT = 3
} EJSPrimStringType;

#define EJS_PRIMSTR_TYPE_MASK 0x07
#define EJS_PRIMSTR_TYPE_MASK_SHIFTED (EJS_PRIMSTR_TYPE_MASK << EJS_GC_USER_FLAGS_SHIFT)

#define EJS_PRIMSTR_SET_TYPE(s,t) (((EJSObject*)(s))->gc_header |= (t) << EJS_GC_USER_FLAGS_SHIFT)
#define EJS_PRIMSTR_CLEAR_TYPE(s) (((EJSObject*)(s))->gc_header &= ~EJS_PRIMSTR_TYPE_MASK_SHIFTED)

#define EJS_PRIMSTR_GET_TYPE(s)   ((EJSPrimStringType)((((EJSPrimString*)(s))->gc_header & EJS_PRIMSTR_TYPE_MASK_SHIFTED) >> EJS_GC_USER_FLAGS_SHIFT))

#define EJS_PRIMSTR_HAS_HASH_MASK 0x08
#define EJS_PRIMSTR_HAS_HASH_MASK_SHIFTED (EJS_PRIMSTR_HAS_HASH_MASK << EJS_GC_USER_FLAGS_SHIFT)
#define EJS_PRIMSTR_HAS_HASH(s) ((((EJSPrimString*)(s))->gc_header & EJS_PRIMSTR_HAS_HASH_MASK_SHIFTED) >> EJS_GC_USER_FLAGS_SHIFT) != 0
#define EJS_PRIMSTR_SET_HAS_HASH(s) ((((EJSPrimString*)(s))->gc_header |= EJS_PRIMSTR_HAS_HASH_MASK_SHIFTED))

typedef struct _EJSPrimString {
    GCObjectHeader gc_header;
    uint32_t length;
    int32_t hash;
    union {
        // utf8 \0 terminated
        //    for flattened strings, this points to the memory location just beyond this struct - i.e. (char*)primStringPointer + sizeof(_EJSPrimString)
        //    for atoms/string literals, this points to the statically compiled C string constant.
        jschar *flat;
        struct {
            struct _EJSPrimString *left;
            struct _EJSPrimString *right;
        } rope;
        struct {
            struct _EJSPrimString *dep;
            int off;
        } dependent;
    } data;
}EJSPrimString;


ejsval _ejs_string_new_utf8 (const char* str);
ejsval _ejs_string_new_utf8_len (const char* str, int len);
ejsval _ejs_string_new_ucs2 (const uint16_t* str);
ejsval _ejs_string_new_ucs2_len (const uint16_t* str, int len);
ejsval _ejs_string_new_substring (ejsval str, int off, int len);
ejsval _ejs_string_new_len(const jschar* str, int len);

ejsval _ejs_string_concat (ejsval left, ejsval right);
ejsval _ejs_string_concatv (ejsval first, ...);
EJSPrimString* _ejs_string_flatten (ejsval str);
EJSPrimString* _ejs_primstring_flatten (EJSPrimString* primstr);

uint16_t _ejs_string_at (EJSPrimString* primstr, uint32_t offset);

uint32_t hash6432shift(uint64_t key);
uint32_t _ejs_string_hash (ejsval str);

char* _ejs_string_to_utf8(EJSPrimString* primstr);

void _ejs_string_init_literal (const char *name, ejsval *val, EJSPrimString* str, jschar* ucs2_data, int32_t length);

#define EJSVAL_IS_STRINGITERATOR(v) (EJSVAL_IS_OBJECT(v) && (EJSVAL_TO_OBJECT(v)->ops == &_ejs_StringIterator_specops))

typedef struct {
    /* object header */
    EJSObject obj;

    ejsval iterated;
    int next_index;
} EJSStringIterator;

extern ejsval _ejs_StringIterator;
extern ejsval _ejs_StringIterator_prototype;
extern EJSSpecOps _ejs_StringIterator_specops;

ejsval _ejs_string_iterator_new(ejsval array);

EJS_END_DECLS;

#endif /* _ejs_string_h */

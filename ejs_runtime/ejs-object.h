/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#ifndef _ejs_object_h_
#define _ejs_object_h_

#include "ejs-value.h"
#include "ejs-gc.h"

// really terribly performing property maps
typedef struct {
#define EJS_PROP_FLAGS_SET_MASK (EJS_PROP_FLAGS_ENUMERABLE_SET | EJS_PROP_FLAGS_CONFIGURABLE_SET | EJS_PROP_FLAGS_WRITABLE_SET | EJS_PROP_FLAGS_VALUE_SET | EJS_PROP_FLAGS_GETTER_SET | EJS_PROP_FLAGS_SETTER_SET)

#define EJS_PROP_ENUMERABLE   (EJS_PROP_FLAGS_ENUMERABLE | EJS_PROP_FLAGS_ENUMERABLE_SET)
#define EJS_PROP_CONFIGURABLE (EJS_PROP_FLAGS_CONFIGURABLE | EJS_PROP_FLAGS_CONFIGURABLE_SET)
#define EJS_PROP_WRITABLE     (EJS_PROP_FLAGS_WRITABLE | EJS_PROP_FLAGS_WRITABLE_SET)
#define EJS_PROP_NOT_ENUMERABLE   (EJS_PROP_FLAGS_ENUMERABLE_SET)
#define EJS_PROP_NOT_CONFIGURABLE (EJS_PROP_FLAGS_CONFIGURABLE_SET)
#define EJS_PROP_NOT_WRITABLE     (EJS_PROP_FLAGS_WRITABLE_SET)

    uint32_t flags;
    ejsval value;
    ejsval getter;
    ejsval setter;
} EJSPropertyDesc;

EJSPropertyDesc* _ejs_propertydesc_new ();
void _ejs_propertydesc_free (EJSPropertyDesc* desc);

#define _ejs_property_desc_set_flag(p, v, propflag, flagset) EJS_MACRO_START \
    if ((v)) {                                                          \
        (p)->flags |= (propflag);                                       \
    }                                                                   \
    else {                                                              \
        (p)->flags &= ~(propflag);                                      \
    }                                                                   \
    (p)->flags |= (flagset);                                            \
    EJS_MACRO_END

#define _ejs_property_desc_set_value_flag(p, v, vname, flagset) EJS_MACRO_START \
    (p)->vname = v;                                                     \
    (p)->flags |= (flagset);                                            \
    EJS_MACRO_END

#define _ejs_property_desc_clear_flag(p, flagclear) EJS_MACRO_START     \
    (p)->flags &= ~(flagclear);                                         \
    EJS_MACRO_END

#define _ejs_property_desc_clear_value_flag(p, v, vname, flagclear) EJS_MACRO_START \
    (p)->vname = v;                                                     \
    (p)->flags &= ~(flagclear);                                         \
    EJS_MACRO_END

#define FLAG_TO_BOOL(x) ((x) != 0 ? EJS_TRUE : EJS_FALSE)

#define _ejs_property_desc_set_enumerable(p, v) _ejs_property_desc_set_flag(p, v, EJS_PROP_FLAGS_ENUMERABLE, EJS_PROP_FLAGS_ENUMERABLE_SET)
#define _ejs_property_desc_set_configurable(p, v) _ejs_property_desc_set_flag(p, v, EJS_PROP_FLAGS_CONFIGURABLE, EJS_PROP_FLAGS_CONFIGURABLE_SET)
#define _ejs_property_desc_set_writable(p, v) _ejs_property_desc_set_flag(p, v, EJS_PROP_FLAGS_WRITABLE, EJS_PROP_FLAGS_WRITABLE_SET)
#define _ejs_property_desc_set_value(p, v) _ejs_property_desc_set_value_flag(p, v, value, EJS_PROP_FLAGS_VALUE_SET)
#define _ejs_property_desc_set_getter(p, v) _ejs_property_desc_set_value_flag(p, v, getter, EJS_PROP_FLAGS_GETTER_SET)
#define _ejs_property_desc_set_setter(p, v) _ejs_property_desc_set_value_flag(p, v, setter, EJS_PROP_FLAGS_SETTER_SET)

#define _ejs_property_desc_clear_value(p) _ejs_property_desc_clear_value_flag(p, _ejs_undefined, value, EJS_PROP_FLAGS_VALUE_SET)
#define _ejs_property_desc_clear_writable(p) _ejs_property_desc_clear_flag(p, EJS_PROP_FLAGS_WRITABLE_SET)

#define _ejs_property_desc_clear_getter(p) _ejs_property_desc_clear_value_flag(p, _ejs_undefined, getter, EJS_PROP_FLAGS_GETTER_SET)
#define _ejs_property_desc_clear_setter(p) _ejs_property_desc_clear_value_flag(p, _ejs_undefined, setter, EJS_PROP_FLAGS_SETTER_SET)

#define _ejs_property_desc_has_enumerable(p) FLAG_TO_BOOL((p)->flags & EJS_PROP_FLAGS_ENUMERABLE_SET)
#define _ejs_property_desc_has_configurable(p) FLAG_TO_BOOL((p)->flags & EJS_PROP_FLAGS_CONFIGURABLE_SET)
#define _ejs_property_desc_has_writable(p) FLAG_TO_BOOL((p)->flags & EJS_PROP_FLAGS_WRITABLE_SET)
#define _ejs_property_desc_has_value(p) FLAG_TO_BOOL((p)->flags & EJS_PROP_FLAGS_VALUE_SET)
#define _ejs_property_desc_has_getter(p) FLAG_TO_BOOL((p)->flags & EJS_PROP_FLAGS_GETTER_SET)
#define _ejs_property_desc_has_setter(p) FLAG_TO_BOOL((p)->flags & EJS_PROP_FLAGS_SETTER_SET)

#define _ejs_property_desc_get_flag_default(p,f,fs,def) (((p)->flags & (fs)) != 0 ? FLAG_TO_BOOL((p)->flags & (f)) : (def))
#define _ejs_property_desc_get_value_flag_default(p,v,fs,def) (((p)->flags & (fs)) != 0 ? (p)->v : (def))

#define _ejs_property_desc_is_enumerable(p)   _ejs_property_desc_get_flag_default(p, EJS_PROP_FLAGS_ENUMERABLE, EJS_PROP_FLAGS_ENUMERABLE_SET, EJS_FALSE)
#define _ejs_property_desc_is_configurable(p) _ejs_property_desc_get_flag_default(p, EJS_PROP_FLAGS_CONFIGURABLE, EJS_PROP_FLAGS_CONFIGURABLE_SET, EJS_FALSE)
#define _ejs_property_desc_is_writable(p)     _ejs_property_desc_get_flag_default(p, EJS_PROP_FLAGS_WRITABLE, EJS_PROP_FLAGS_WRITABLE_SET, EJS_FALSE)

#define _ejs_property_desc_get_value(p) _ejs_property_desc_get_value_flag_default(p, value, EJS_PROP_FLAGS_VALUE_SET, _ejs_undefined)
#define _ejs_property_desc_get_getter(p) _ejs_property_desc_get_value_flag_default(p, getter, EJS_PROP_FLAGS_GETTER_SET, _ejs_undefined)
#define _ejs_property_desc_get_setter(p) _ejs_property_desc_get_value_flag_default(p, setter, EJS_PROP_FLAGS_SETTER_SET, _ejs_undefined)

ejsval  ToPropertyKey          (ejsval argument);
void    ToPropertyDescriptor   (ejsval O, EJSPropertyDesc* desc);
ejsval  FromPropertyDescriptor (EJSPropertyDesc* Desc);
EJSBool IsDataDescriptor       (EJSPropertyDesc* Desc);
EJSBool IsAccessorDescriptor   (EJSPropertyDesc* Desc);

ejsval Get (ejsval O, ejsval P);
ejsval Put (ejsval O, ejsval P, ejsval V, EJSBool Throw);
ejsval GetMethod (ejsval O, ejsval P);
    
typedef struct _EJSPropertyMapEntry {
    // the next entry in this bucket
    struct _EJSPropertyMapEntry *next_bucket;

    // the next entry in insertion order
    struct _EJSPropertyMapEntry *next_insert;

    ejsval name;
    EJSPropertyDesc* desc;
}_EJSPropertyMapEntry;

typedef struct EJSPropertyMap {
    _EJSPropertyMapEntry* head_insert;
    _EJSPropertyMapEntry* tail_insert;
    _EJSPropertyMapEntry** buckets;
    int nbuckets;
    int inuse;
}EJSPropertyMap;

typedef struct EJSSpecOps EJSSpecOps;
typedef struct EJSObject {
    GCObjectHeader     gc_header;
    EJSSpecOps*      ops;

    // if lsb is 0 we haven't calculated it yet
    uint32_t hashcode;
    //use ref_count to replace garbage collector
    uint32_t ref_count;
    
    ejsval        proto; // [[Prototype]]
    EJSPropertyMap  map;
}EJSObject;

typedef void (*EJSPropertyDescFunc)(ejsval name, EJSPropertyDesc *desc, void* data);
typedef void (*EJSValueFunc)(ejsval value);

typedef ejsval           (*SpecOpGetPrototypeOf) (ejsval obj);
typedef EJSBool          (*SpecOpSetPrototypeOf) (ejsval obj, ejsval proto);
typedef ejsval           (*SpecOpGet) (ejsval obj, ejsval propertyName, ejsval receiver);
typedef EJSPropertyDesc* (*SpecOpGetOwnProperty) (ejsval obj, ejsval propertyName, ejsval* exc);
typedef EJSBool          (*SpecOpSet) (ejsval obj, ejsval propertyName, ejsval val, ejsval receiver);
typedef EJSBool          (*SpecOpHasProperty) (ejsval obj, ejsval propertyName);
typedef EJSBool          (*SpecOpDelete) (ejsval obj, ejsval propertyName, EJSBool flag);
typedef ejsval           (*SpecOpDefaultValue) (ejsval obj, const char *hint);
typedef EJSBool          (*SpecOpDefineOwnProperty) (ejsval obj, ejsval propertyName, EJSPropertyDesc* propertyDescriptor, EJSBool _throw);

typedef EJSObject*       (*SpecOpAllocate) ();
typedef void             (*SpecOpFinalize) (EJSObject* obj);
typedef void             (*SpecOpScan) (EJSObject* obj, EJSValueFunc scan_func);

typedef EJSBool          (*SpecOpIsExtensible) (ejsval obj);
typedef EJSBool          (*SpecOpPreventExtensions) (ejsval obj);
typedef ejsval           (*SpecOpEnumerate) (ejsval obj);
typedef ejsval           (*SpecOpOwnPropertyKeys) (ejsval obj);

typedef struct EJSSpecOps {
    // special ops defined in the standard
    const char* class_name;

    SpecOpGetPrototypeOf GetPrototypeOf;
    SpecOpSetPrototypeOf SetPrototypeOf;

    SpecOpIsExtensible IsExtensible;
    SpecOpPreventExtensions PreventExtensions;

    SpecOpGetOwnProperty GetOwnProperty;
    SpecOpDefineOwnProperty DefineOwnProperty;
    SpecOpHasProperty HasProperty;

    SpecOpGet Get;
    SpecOpSet Set;
    SpecOpDelete Delete;
    SpecOpEnumerate Enumerate;
    SpecOpOwnPropertyKeys OwnPropertyKeys;

    // ejs-defined ops
    SpecOpAllocate Allocate;
    SpecOpFinalize Finalize; // called when there are no remaining references to this object
    SpecOpScan     Scan;     // used to enumerate object references
}EJSSpecOps;

#define OP_INHERIT (void*)-1
#define EJS_DEFINE_CLASS(n, get_prototype_of, set_prototype_of, is_extensible, prevent_extensions, get_own_property, define_own_property, has_property, get, set, _delete, enumerate, own_property_keys, allocate, finalize, scan) \
    EJSSpecOps _ejs_##n##_specops = {                                   \
        #n,                                               \
        (SpecOpGetPrototypeOf)(get_prototype_of),                           \
        (SpecOpSetPrototypeOf)(set_prototype_of),                           \
        (SpecOpIsExtensible)(is_extensible),                                \
        (SpecOpPreventExtensions)(prevent_extensions),                      \
        (SpecOpGetOwnProperty)(get_own_property),                           \
        (SpecOpDefineOwnProperty)(define_own_property),                     \
        (SpecOpHasProperty)(has_property),                                  \
        (SpecOpGet)(get),                                                   \
        (SpecOpSet)(set),                                                   \
        (SpecOpDelete)(_delete),                                            \
        (SpecOpEnumerate)(enumerate),                                       \
        (SpecOpOwnPropertyKeys)(own_property_keys),                         \
        (SpecOpAllocate)(allocate),                                         \
        (SpecOpFinalize)(finalize),                                         \
        (SpecOpScan)(scan)                                                  \
    };
#define EJS_DEFINE_INHERIT_ALL_CLASS(n) EJS_DEFINE_CLASS(n, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT, OP_INHERIT)


void _ejs_Class_initialize (EJSSpecOps *child, EJSSpecOps* parent);

#define EJS_OBJECT_EXTENSIBLE_FLAG 0x01

#define EJS_OBJECT_EXTENSIBLE_FLAG_SHIFTED (EJS_OBJECT_EXTENSIBLE_FLAG << EJS_GC_USER_FLAGS_SHIFT)

#define EJS_OBJECT_SET_EXTENSIBLE(o) (((EJSObject*)(o))->gc_header |= EJS_OBJECT_EXTENSIBLE_FLAG_SHIFTED)
#define EJS_OBJECT_CLEAR_EXTENSIBLE(o) (((EJSObject*)(o))->gc_header &= ~EJS_OBJECT_EXTENSIBLE_FLAG_SHIFTED)

#define EJS_OBJECT_IS_EXTENSIBLE(o) ((((EJSObject*)(o))->gc_header & EJS_OBJECT_EXTENSIBLE_FLAG_SHIFTED) != 0)

EJS_BEGIN_DECLS;

#define OP(o,op) EJS_ASSERT_VAL(o, "object is null in call to " #op " op", (((EJSObject*)o)->ops->op))
ejsval _ejs_specop_get_prototype_of(ejsval obj);
EJSBool  _ejs_specop_set_prototype_of(ejsval obj, ejsval proto);
EJSBool  _ejs_specop_is_extensible(ejsval obj);
EJSBool  _ejs_specop_preventextensions(ejsval obj);
EJSPropertyDesc *_ejs_specop_get_own_property(ejsval obj, ejsval propertyName, ejsval* exc);
EJSBool  _ejs_specop_define_own_property(ejsval obj, ejsval propertyName, EJSPropertyDesc* Desc, EJSBool Throw);
EJSBool  _ejs_specop_has_property(ejsval obj, ejsval propertyName);
ejsval _ejs_specop_get(ejsval obj, ejsval propertyName, ejsval Receiver);
EJSBool  _ejs_specop_set(ejsval obj, ejsval propertyName, ejsval value, ejsval Receiver);
EJSBool  _ejs_specop_delete(ejsval obj, ejsval propertyName, EJSBool Throw);
ejsval _ejs_specop_enumerate(ejsval obj);
ejsval _ejs_specop_own_property_keys(ejsval obj);
EJSObject *_ejs_specop_allocate(EJSObject *jsObj);
void _ejs_specop_finalize(EJSObject *jsObj);
void _ejs_specop_scan(EJSObject *jsObj, EJSValueFunc scan_func);
#define OP_class_name(jsobj_this) (((EJSObject*)jsobj_this)->ops->class_name)

#define CLASSNAME(o) OP(o,class_name)

void _ejs_propertymap_init (EJSPropertyMap* map);
EJSPropertyDesc* _ejs_propertymap_lookup (EJSPropertyMap *map, ejsval name);
void _ejs_propertymap_insert (EJSPropertyMap *map, ejsval name, EJSPropertyDesc* desc);
void _ejs_propertymap_remove (EJSPropertyMap *map, ejsval name);
void _ejs_propertymap_foreach_value (EJSPropertyMap *map, EJSValueFunc foreach_func);
void _ejs_propertymap_foreach_property (EJSPropertyMap *map, EJSPropertyDescFunc foreach_func, void* data);

EJSBool _ejs_object_define_value_property (ejsval obj, ejsval key, ejsval value, uint32_t flags);
EJSBool _ejs_object_define_setter_property (ejsval obj, ejsval key, ejsval set, uint32_t flags);
EJSBool _ejs_object_define_getter_property (ejsval obj, ejsval key, ejsval get, uint32_t flags);

ejsval _ejs_object_setprop (ejsval obj, ejsval key, ejsval value);
ejsval _ejs_object_getprop (ejsval obj, ejsval key);

ejsval _ejs_global_setprop (ejsval key, ejsval value);
ejsval _ejs_global_getprop (ejsval key);

ejsval _ejs_object_setprop_utf8 (ejsval obj, const char *key, ejsval value);
ejsval _ejs_object_getprop_utf8 (ejsval obj, const char *key);

ejsval  _ejs_property_iterator_new (ejsval forObj);
ejsval  _ejs_property_iterator_current (ejsval iterator);
EJSBool _ejs_property_iterator_next (ejsval iterator, EJSBool free_on_end);
void    _ejs_property_iterator_free (ejsval iterator);

extern ejsval _ejs_Object;
extern ejsval _ejs_Object__proto__;
extern ejsval _ejs_Object_prototype;
extern EJSSpecOps _ejs_Object_specops;
extern EJSSpecOps _ejs__EJSPropertyIterator_specops;

void _ejs_object_init_proto();

ejsval _ejs_object_new  (ejsval proto, EJSSpecOps* ops);
void   _ejs_init_object (EJSObject *obj, ejsval proto, EJSSpecOps *ops);

void _ejs_object_finalize(EJSObject *obj);

void _ejs_object_init(ejsval global);
uint32_t _ejs_object_hash (ejsval obj);

// we shouldn't expose this method, we should expose a helper method that calls this.
ejsval _ejs_Object_prototype_toString (ejsval env, ejsval _this, uint32_t argc, ejsval *args);

// exposed so we can call the native implementation during class creation
ejsval _ejs_object_set_prototype_of (ejsval obj, ejsval proto);

ejsval _ejs_object_create (ejsval proto);
ejsval _ejs_Object_create (ejsval env, ejsval _this, uint32_t argc, ejsval *args);
ejsval _ejs_Object_getOwnPropertyNames (ejsval env, ejsval _this, uint32_t argc, ejsval *args);

void _ejs_Object_init (ejsval ejs_global);
void _ejs_dump_value (ejsval val);

ejsval _ejs_number_new (double value);

void _ejs_value_finalize(ejsval val);

uint32_t JSValueHash(ejsval key);
ejsval _ejs_eval (ejsval env, ejsval _this, uint32_t argc, ejsval* args);

EJS_END_DECLS;

#endif // _ejs_object_h_

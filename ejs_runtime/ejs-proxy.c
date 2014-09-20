/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#include "ejs-proxy.h"
#include "ejs-gc.h"
#include "ejs-error.h"
#include "ejs-function.h"
#include "ejs-ops.h"
#include "ejs-symbol.h"

ejsval
_ejs_Proxy_create (ejsval env, ejsval _this, uint32_t argc, ejsval* args)
{
    if (argc == 0)
        _ejs_throw_nativeerror_utf8(EJS_TYPE_ERROR, "create requires more than 0 arguments");
    if (!EJSVAL_IS_OBJECT(args[0]))
        _ejs_throw_nativeerror_utf8(EJS_TYPE_ERROR, "argument 0 is not a non-null object");

    EJS_NOT_IMPLEMENTED();
	return _ejs_null;
}

ejsval
_ejs_Proxy_createFunction (ejsval env, ejsval _this, uint32_t argc, ejsval* args)
{
    if (argc <= 1)
        _ejs_throw_nativeerror_utf8(EJS_TYPE_ERROR, "create requires more than 1 arguments");
    if (!EJSVAL_IS_OBJECT(args[0]))
        _ejs_throw_nativeerror_utf8(EJS_TYPE_ERROR, "argument 0 is not a non-null object");
    if (!EJSVAL_IS_FUNCTION(args[1]))
        _ejs_throw_nativeerror_utf8(EJS_TYPE_ERROR, "argument 1 is not a function");
    if (argc > 2 && !EJSVAL_IS_FUNCTION(args[2]))
        _ejs_throw_nativeerror_utf8(EJS_TYPE_ERROR, "argument 2 is not a function");

    EJS_NOT_IMPLEMENTED();
	return _ejs_null;
}

// ECMA262: 26.5.1 The Proxy Constructor Function
// Proxy ( target, handler ) 
static ejsval
_ejs_Proxy_impl (ejsval env, ejsval _this, uint32_t argc, ejsval *args)
{
    if (EJSVAL_IS_UNDEFINED(_this)) // not called as a constructor
        _ejs_throw_nativeerror_utf8(EJS_TYPE_ERROR, "Proxy cannot be called as a function");

    ejsval target = _ejs_undefined;
    ejsval handler = _ejs_undefined;

    if (argc > 0) target = args[0];
    if (argc > 1) handler = args[1];

    // 1. If Type(target) is not Object, throw a TypeError Exception. 
    if (!EJSVAL_IS_OBJECT(target))
        _ejs_throw_nativeerror_utf8(EJS_TYPE_ERROR, "Proxy target argument must be an object");

    // 2. If Type(handler) is not Object, throw a TypeError Exception. 
    if (!EJSVAL_IS_OBJECT(handler))
        _ejs_throw_nativeerror_utf8(EJS_TYPE_ERROR, "Proxy handler argument must be an object");

    // 3. Let P be a newly created object. 
    // 4. Set P’s essential internal methods to the definitions specified in 9.5. 
    // 5. If IsCallable(target) is true, then 
    if (EJSVAL_IS_CALLABLE(target)) {
        //    a. Set the [[Call]] internal method of P as specified in 9.5.13. 
        //    b. If target has a [[Construct]] internal method, then 
        //       i. Set the [[Construct]] internal method of P as specified in 9.5.14. 

        _ejs_log("proxies don't save off [[Call]]/[[Construct]] presence yet.");
    }
    EJSProxy* P = EJSVAL_TO_PROXY(_this);

    // 6. Set the [[ProxyTarget]] internal slot of P to target. 
    P->target = target;
    
    // 7. Set the [[ProxyHandler]] internal slot of P to handler. 
    P->handler = handler;

    // 8. Return P. 
    return _this;
}

ejsval _ejs_Proxy EJSVAL_ALIGNMENT;
ejsval _ejs_Proxy_prototype EJSVAL_ALIGNMENT;

static ejsval
_ejs_Proxy_create_impl (ejsval env, ejsval _this, uint32_t argc, ejsval *args)
{
    // 1. Let F be the this value. 
    ejsval F = _this;

    if (!EJSVAL_IS_CONSTRUCTOR(F)) 
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "'this' in Proxy[Symbol.create] is not a constructor");

    // 2. Let obj be the result of calling OrdinaryCreateFromConstructor(F, "%ObjectPrototype%", ([[ProxyTarget]], [[ProxyHandler]]) ). 
    ejsval proto = _ejs_specop_get(F, _ejs_atom_prototype, F);
    if (EJSVAL_IS_UNDEFINED(proto))
        proto = _ejs_Object_prototype;

    EJSObject* obj = (EJSObject*)_ejs_gc_new (EJSProxy);
    _ejs_init_object (obj, proto, &_ejs_Proxy_specops);
    return OBJECT_TO_EJSVAL(obj);
}

void
_ejs_proxy_init(ejsval global)
{
    _ejs_Proxy = _ejs_function_new_without_proto (_ejs_null, _ejs_atom_Proxy, (EJSClosureFunc)_ejs_Proxy_impl);
    _ejs_object_setprop (global, _ejs_atom_Proxy, _ejs_Proxy);

    // the spec doesn't mention a prototype (and other implementations
    // don't have one, but we need one in order to associate an
    // instance with its specops in _ejs_object_create.)
    _ejs_gc_add_root (&_ejs_Proxy_prototype);
    _ejs_Proxy_prototype = _ejs_object_new(_ejs_null, &_ejs_Object_specops);
    _ejs_object_setprop (_ejs_Proxy, _ejs_atom_prototype, _ejs_Proxy_prototype);

#define OBJ_METHOD(x) EJS_INSTALL_ATOM_FUNCTION(_ejs_Proxy, x, _ejs_Proxy_##x)

    OBJ_METHOD(create);
    OBJ_METHOD(createFunction);

#undef OBJ_METHOD

    EJS_INSTALL_SYMBOL_FUNCTION_FLAGS (_ejs_Proxy, create, _ejs_Proxy_create_impl, EJS_PROP_NOT_ENUMERABLE);
}


static ejsval
_ejs_proxy_specop_get_prototype_of (ejsval O)
{
    EJSProxy* proxy = EJSVAL_TO_PROXY(O);

    // 1. Let handler be the value of the [[ProxyHandler]] internal slot of O.
    ejsval handler = proxy->handler;

    // 2. If handler is null, then throw a TypeError exception. 
    if (EJSVAL_IS_NULL(handler))
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "null ProxyHandler in getPrototypeOf");
        
    // 3. Let target be the value of the [[ProxyTarget]] internal slot of O. 
    ejsval target = proxy->target;

    // 4. Let trap be GetMethod(handler, "getPrototypeOf").
    // 5. ReturnIfAbrupt(trap). 
    ejsval trap = GetMethod(handler, _ejs_atom_getPrototypeOf);

    // 6. If trap is undefined, then 
    if (EJSVAL_IS_UNDEFINED(trap)) {
        //    a. Return the result of calling the [[GetPrototypeOf]] internal method of target. 
        return _ejs_specop_get_prototype_of(target);
    }

    // 7. Let handlerProto be the result of calling the [[Call]] internal method of trap with handler as the this value and a new List containing target.
    // 8. ReturnIfAbrupt(handlerProto). 
    ejsval args[] = { target };

    ejsval handlerProto = _ejs_invoke_closure(trap, handler, 1, args);
    
    // 9. If Type(handlerProto) is neither Object nor Null, then throw a TypeError exception. 
    if (!EJSVAL_IS_OBJECT(handlerProto) && EJSVAL_IS_NULL(handlerProto))
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "getPrototypeOf trap returned invalid prototype");
        
    EJSObject* _target = EJSVAL_TO_OBJECT(target); // XXX do we need to verify that it actually is an object here?
    // 10. Let extensibleTarget be IsExtensible(target). 
    // 11. ReturnIfAbrupt(extensibleTarget). 
    EJSBool extensibleTarget = EJS_OBJECT_IS_EXTENSIBLE(_target);

    // 12. If extensibleTarget is true, then return handlerProto.
    if (extensibleTarget)
        return handlerProto;

    // 13. Let targetProto be the result of calling the [[GetPrototypeOf]] internal method of target. 
    // 14. ReturnIfAbrupt(targetProto).
    ejsval targetProto = _ejs_specop_get_prototype_of(target);

    // 15. If SameValue(handlerProto, targetProto) is false, then throw a TypeError exception. 
    if (!SameValue(handlerProto, targetProto))
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "inconsistent prototypes from target and handler");

    // 16. Return handlerProto.
    return handlerProto;
}

static EJSBool
_ejs_proxy_specop_set_prototype_of (ejsval O, ejsval V)
{
    EJSProxy* proxy = EJSVAL_TO_PROXY(O);
    // 1. Assert: Either Type(V) is Object or Type(V) is Null. 

    // 2. Let handler be the value of the [[ProxyHandler]] internal slot of O. 
    ejsval handler = proxy->handler;

    // 3. If handler is null, then throw a TypeError exception. 
    if (EJSVAL_IS_NULL(handler))
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "null ProxyHandler in setPrototypeOf");

    // 4. Let target be the value of the [[ProxyTarget]] internal slot of O. 
    ejsval target = proxy->target;

    EJSObject* _target = EJSVAL_TO_OBJECT(target);

    // 5. Let trap be GetMethod(handler, "setPrototypeOf"). 
    // 6. ReturnIfAbrupt(trap). 
    ejsval trap = GetMethod(handler, _ejs_atom_setPrototypeOf);

    // 7. If trap is undefined, then 
    if (EJSVAL_IS_UNDEFINED(trap)) {
        // a. Return the result of calling the [[SetPrototypeOf]] internal method of target with argument V. 
        return _ejs_specop_set_prototype_of(target, V);
    }
    // 8. Let trapResult be the result of calling the [[Call]] internal method of trap with handler as the this value and a new List containing target and V. 
    ejsval args[] = { target, V };
    ejsval trapResult = _ejs_invoke_closure(trap, handler, 2, args);

    // 9. Let booleanTrapResult be ToBoolean(trapResult). 
    // 10. ReturnIfAbrupt(booleanTrapResult). 
    EJSBool booleanTrapResult = ToEJSBool(trapResult);

    // 11. Let extensibleTarget be IsExtensible(target). 
    // 12. ReturnIfAbrupt(extensibleTarget). 
    EJSBool extensibleTarget = EJS_OBJECT_IS_EXTENSIBLE(_target);

    // 13. If extensibleTarget is true, then return booleanTrapResult. 
    if (extensibleTarget)
        return booleanTrapResult;

    // 14. Let targetProto be the result of calling the [[GetPrototypeOf]] internal method of target. 
    // 15. ReturnIfAbrupt(targetProto). 
    ejsval targetProto = _ejs_specop_get_prototype_of(target);

    // 16. If booleanTrapResult is true and SameValue(V, targetProto) is false, then throw a TypeError exception. 
    if (booleanTrapResult && SameValue(V, targetProto)) {
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "cycle detected in prototype chain");
    }
    // 17. Return booleanTrapResult. 
    return booleanTrapResult;
}

static ejsval
_ejs_proxy_specop_get (ejsval obj, ejsval propertyName, ejsval receiver)
{
    EJSProxy* proxy = EJSVAL_TO_PROXY(obj);

    // 1. Assert: IsPropertyKey(P) is true. 
    // 2. Let handler be the value of the [[ProxyHandler]] internal slot of O. 
    ejsval handler = proxy->handler;

    // 3. If handler is null, then throw a TypeError exception. 
    if (EJSVAL_IS_NULL(handler))
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "null ProxyHandler in get");

    // 4. Let target be the value of the [[ProxyTarget]] internal slot of O. 
    ejsval target = proxy->target;

    EJSObject* _target = EJSVAL_TO_OBJECT(target);

    // 5. Let trap be GetMethod(handler, "get"). 
    // 6. ReturnIfAbrupt(trap). 
    ejsval trap = GetMethod(handler, _ejs_atom_get);

    // 7. If trap is undefined, then 
    if (EJSVAL_IS_UNDEFINED(trap)) {
        //    a. Return the result of calling the [[Get]] internal method of target with arguments P and Receiver. 
        return _ejs_specop_get(target, propertyName, receiver);
    }

    // 8. Let trapResult be the result of calling the [[Call]] internal method of trap with handler as the this value and a new List containing target, P, and Receiver. 
    // 9. ReturnIfAbrupt(trapResult). 
    ejsval args[] = { target, propertyName, receiver };
    ejsval trapResult = _ejs_invoke_closure(trap, handler, 3, args);

    // 10. Let targetDesc be the result of calling the [[GetOwnProperty]] internal method of target with argument P.
    // 11. ReturnIfAbrupt(targetDesc). 
    EJSPropertyDesc* targetDesc = _ejs_specop_get_own_property(target, propertyName, NULL);

    // 12. If targetDesc is not undefined, then 
    if (!targetDesc) {
        //     a. If IsDataDescriptor(targetDesc) and targetDesc.[[Configurable]] is false and targetDesc.[[Writable]] is false, then 
        if (IsDataDescriptor (targetDesc) && !_ejs_property_desc_is_configurable(targetDesc) && !_ejs_property_desc_is_writable(targetDesc)) {
            //        i. If SameValue(trapResult, targetDesc.[[Value]]) is false, then throw a TypeError exception. 
            if (!SameValue(trapResult, _ejs_property_desc_get_value(targetDesc)))
                _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "1"); // XXX
        }
        //     b. If IsAccessorDescriptor(targetDesc) and targetDesc.[[Configurable]] is false and targetDesc.[[Get]] is undefined, then 
        if (IsAccessorDescriptor (targetDesc) && !_ejs_property_desc_is_configurable(targetDesc) && EJSVAL_IS_UNDEFINED(_ejs_property_desc_get_getter(targetDesc))) {
            //        i. If trapResult is not undefined, then throw a TypeError exception. 
            if (!EJSVAL_IS_UNDEFINED(trapResult))
                _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "2"); // XXX
        }
    }
    // 13. Return trapResult. 
    return trapResult;
}

static EJSPropertyDesc*
_ejs_proxy_specop_get_own_property (ejsval O, ejsval P, ejsval* exc)
{
    // 1. Assert: IsPropertyKey(P) is true. 
    EJSProxy* proxy = EJSVAL_TO_PROXY(O);

    // 2. Let handler be the value of the [[ProxyHandler]] internal slot of O. 
    ejsval handler = proxy->handler;

    // 3. If handler is null, then throw a TypeError exception. 
    if (EJSVAL_IS_NULL(handler))
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "null ProxyHandler in getOwnProperty");

    // 4. Let target be the value of the [[ProxyTarget]] internal slot of O. 
    ejsval target = proxy->target;

    EJSObject* _target = EJSVAL_TO_OBJECT(target);

    // 5. Let trap be GetMethod(handler, "getOwnPropertyDescriptor"). 
    // 6. ReturnIfAbrupt(trap). 
    ejsval trap = GetMethod(handler, _ejs_atom_getOwnPropertyDescriptor);

    // 7. If trap is undefined, then 
    if (EJSVAL_IS_UNDEFINED(trap)) {
        //    a. Return the result of calling the [[GetOwnProperty]] internal method of target with argument P.
        return _ejs_specop_get_own_property(target, P, NULL);
    }

    // 8. Let trapResultObj be the result of calling the [[Call]] internal method of trap with handler as the this value and a new List containing target and P. 
    // 9. ReturnIfAbrupt(trapResultObj). 
    ejsval args[] = { target, P };
    ejsval trapResultObj = _ejs_invoke_closure(trap, handler, 2, args);

    // 10. If Type(trapResultObj) is neither Object nor Undefined, then throw a TypeError exception. 
    if (!EJSVAL_IS_OBJECT(trapResultObj) && !EJSVAL_IS_UNDEFINED(trapResultObj)) {
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "return value from getOwnProperty trap must be an object or undefined");
    }

    // 11. Let targetDesc be the result of calling the [[GetOwnProperty]] internal method of target with argument P. 
    // 12. ReturnIfAbrupt(targetDesc). 
    EJSPropertyDesc* targetDesc = _ejs_specop_get_own_property(target, P, NULL);

    // 13. If trapResultObj is undefined, then 
    if (EJSVAL_IS_UNDEFINED(trapResultObj)) {
        //     a. If targetDesc is undefined, then return undefined. 
        if (!targetDesc)
            return NULL;
        //     b. If targetDesc.[[Configurable]] is false, then throw a TypeError exception. 
        if (!_ejs_property_desc_is_configurable(targetDesc))
            _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "1"); // XXX

        //     c. Let extensibleTarget be IsExtensible(target). 
        //     d. ReturnIfAbrupt(extensibleTarget). 
        EJSBool extensibleTarget = EJS_OBJECT_IS_EXTENSIBLE(_target);

        //     e. If ToBoolean(extensibleTarget) is false, then throw a TypeError exception. 
        if (!extensibleTarget)
            _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "2"); // XXX

        //     f. Return undefined. 
        return NULL;
    }

    // 14. Let extensibleTarget be IsExtensible(target). 
    // 15. ReturnIfAbrupt(extensibleTarget). 
    EJSBool extensibleTarget = EJS_OBJECT_IS_EXTENSIBLE(_target);

    // 16. Let resultDesc be ToPropertyDescriptor(trapResultObj).
    // 17. ReturnIfAbrupt(resultDesc). 
    EJSPropertyDesc resultDesc;
    ToPropertyDescriptor(trapResultObj, &resultDesc);
    
    // 18. Call CompletePropertyDescriptor(resultDesc, undefined). 
    EJS_NOT_IMPLEMENTED();
    // 19. Let valid be IsCompatiblePropertyDescriptor (extensibleTarget, resultDesc, targetDesc). 
    EJS_NOT_IMPLEMENTED();
    // 20. If valid is false, then throw a TypeError exception. 
    EJS_NOT_IMPLEMENTED();

    // 21. If resultDesc.[[Configurable]] is false, then 
    if (!_ejs_property_desc_is_configurable(&resultDesc)) {
        //     a. If targetDesc is undefined or targetDesc.[[Configurable]] is true, then 
        if (!targetDesc || _ejs_property_desc_is_configurable(targetDesc))
            //        i. Throw a TypeError exception. 
            _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "3"); // XXX
    }

    EJS_NOT_IMPLEMENTED();
	return NULL;
#if 0//false
    // XXX this can't work, since resultDesc is on the stack.

    // 22. Return resultDesc. 
    return resultDesc;
#endif
}

static EJSBool
_ejs_proxy_specop_set (ejsval obj, ejsval propertyName, ejsval val, ejsval receiver)
{
    // 1. Assert: IsPropertyKey(P) is true. 

    EJSProxy* proxy = EJSVAL_TO_PROXY(obj);

    // 2. Let handler be the value of the [[ProxyHandler]] internal slot of O. 
    ejsval handler = proxy->handler;

    // 3. If handler is null, then throw a TypeError exception. 
    if (EJSVAL_IS_NULL(handler)) {
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "1"); // XXX
    }

    // 4. Let target be the value of the [[ProxyTarget]] internal slot of O. 
    ejsval target = proxy->target;

    EJSObject* _target = EJSVAL_TO_OBJECT(target);

    // 5. Let trap be GetMethod(handler, "set"). 
    // 6. ReturnIfAbrupt(trap). 
    ejsval trap = GetMethod(handler, _ejs_atom_set);

    // 7. If trap is undefined, then 
    if (EJSVAL_IS_UNDEFINED(trap)) {
        // a. Return the result of calling the [[Set]] internal method of target with arguments P, V, and Receiver. 
        return _ejs_specop_set(target, propertyName, val, receiver);
    }
    // 8. Let trapResult be the result of calling the [[Call]] internal method of trap with handler as the this value and a new List containing target, P, V, and Receiver. 
    ejsval args[] = { target, propertyName, val, receiver };
    ejsval trapResult = _ejs_invoke_closure(trap, handler, 4, args);

    // 9. Let booleanTrapResult be ToBoolean(trapResult). 
    // 10. ReturnIfAbrupt(booleanTrapResult). 
    EJSBool booleanTrapResult = ToEJSBool(trapResult);

    // 11. If booleanTrapResult is false, then return false. 
    if (!booleanTrapResult)
        return EJS_FALSE;

    // 12. Let targetDesc be the result of calling the [[GetOwnProperty]] internal method of target with argument P. 
    // 13. ReturnIfAbrupt(targetDesc). 
    EJSPropertyDesc* targetDesc = _ejs_specop_get_own_property(target, propertyName, NULL);

    // 14. If targetDesc is not undefined, then 
    if (targetDesc) {
        //     a. If IsDataDescriptor(targetDesc) and targetDesc.[[Configurable]] is false and targetDesc.[[Writable]] is false, then 
        if (IsDataDescriptor(targetDesc) && !_ejs_property_desc_is_configurable(targetDesc) && _ejs_property_desc_is_writable(targetDesc)) {
            //        i. If SameValue(V, targetDesc.[[Value]]) is false, then throw a TypeError exception. 
            if (!SameValue(val, _ejs_property_desc_get_value(targetDesc)))
                _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "2"); // XXX
        }
        //     b. If IsAccessorDescriptor(targetDesc) and targetDesc.[[Configurable]] is false, then 
        if (IsAccessorDescriptor(targetDesc) && !_ejs_property_desc_is_configurable(targetDesc)) {
            //        i. If targetDesc.[[Set]] is undefined, then throw a TypeError exception. 
            if (EJSVAL_IS_UNDEFINED(_ejs_property_desc_get_setter(targetDesc)))
                _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "2"); // XXX
        }
    }
    // 15. Return true. 
    return EJS_TRUE;
}

static EJSBool
_ejs_proxy_specop_has_property (ejsval O, ejsval P)
{
    // 1. Assert: IsPropertyKey(P) is true. 

    EJSProxy* proxy = EJSVAL_TO_PROXY(O);

    // 2. Let handler be the value of the [[ProxyHandler]] internal slot of O. 
    ejsval handler = proxy->handler;

    // 3. If handler is null, then throw a TypeError exception. 
    if (EJSVAL_IS_NULL(handler)) {
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "1"); // XXX
    }
    // 4. Let target be the value of the [[ProxyTarget]] internal slot of O. 
    ejsval target = proxy->target;

    EJSObject* _target = EJSVAL_TO_OBJECT(target);

    // 5. Let trap be GetMethod(handler, "has"). 
    // 6. ReturnIfAbrupt(trap). 
    ejsval trap = GetMethod(handler, _ejs_atom_has);

    // 7. If trap is undefined, then 
    if (EJSVAL_IS_UNDEFINED(trap)) {
        //    a. Return the result of calling the [[HasProperty]] internal method of target with argument P. 
        return _ejs_specop_has_property(target, P);
    }

    // 8. Let trapResult be the result of calling the [[Call]] internal method of trap with handler as the this value and a new List containing target and P. 
    ejsval args[] = { target, P };
    ejsval trapResult = _ejs_invoke_closure(trap, handler, 2, args);

    // 9. Let booleanTrapResult be ToBoolean(trapResult). 
    // 10. ReturnIfAbrupt(booleanTrapResult). 
    EJSBool booleanTrapResult = ToEJSBool(trapResult);

    // 11. If booleanTrapResult is false, then 
    if (!booleanTrapResult) {
        //     a. Let targetDesc be the result of calling the [[GetOwnProperty]] internal method of target with argument P. 
        //     b. ReturnIfAbrupt(targetDesc). 
        EJSPropertyDesc* targetDesc = _ejs_specop_get_own_property(target, P, NULL);

        //     c. If targetDesc is not undefined, then 
        if (!targetDesc) {
            //        i. If targetDesc.[[Configurable]] is false, then throw a TypeError exception. 
            if (!_ejs_property_desc_is_configurable(targetDesc))
                _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "1"); // XXX

            //        ii. Let extensibleTarget be IsExtensible(target). 
            //        iii. ReturnIfAbrupt(extensibleTarget). 

            // XXX

            //        iv. If extensibleTarget is false, then throw a TypeError exception. 
            if (!EJS_OBJECT_IS_EXTENSIBLE(_target))
                _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "2"); // XXX
        }
    }

    // 12. Return booleanTrapResult. 
    return booleanTrapResult;
}

static EJSBool
_ejs_proxy_specop_delete (ejsval O, ejsval P, EJSBool unusedflag)
{
    // 1. Assert: IsPropertyKey(P) is true. 

    EJSProxy* proxy = EJSVAL_TO_PROXY(O);

    // 2. Let handler be the value of the [[ProxyHandler]] internal slot of O. 
    ejsval handler = proxy->handler;

    // 3. If handler is null, then throw a TypeError exception. 
    if (EJSVAL_IS_NULL(handler)) {
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "1"); // XXX
    }

    // 4. Let target be the value of the [[ProxyTarget]] internal slot of O. 
    ejsval target = proxy->target;

    // 5. Let trap be GetMethod(handler, "deleteProperty"). 
    // 6. ReturnIfAbrupt(trap). 
    ejsval trap = GetMethod(handler, _ejs_atom_deleteProperty);

    // 7. If trap is undefined, then 
    if (EJSVAL_IS_UNDEFINED(trap)) {
        //    a. Return the result of calling the [[Delete]] internal method of target with argument P. 
        return _ejs_specop_delete(target, P, unusedflag);
    }

    // 8. Let trapResult be the result of calling the [[Call]] internal method of trap with handler as the this value and a new List containing target and P. 
    ejsval args[] = { target, P };
    ejsval trapResult = _ejs_invoke_closure(trap, handler, 2, args);

    // 9. Let booleanTrapResult be ToBoolean(trapResult). 
    // 10. ReturnIfAbrupt(booleanTrapResult). 
    EJSBool booleanTrapResult = ToEJSBool(trapResult);

    // 11. If booleanTrapResult is false, then return false. 
    if (!booleanTrapResult)
        return EJS_FALSE;

    // 12. Let targetDesc be the result of calling the [[GetOwnProperty]] internal method of target with argument P. 
    // 13. ReturnIfAbrupt(targetDesc). 
    EJSPropertyDesc* targetDesc = _ejs_specop_get_own_property(target, P, NULL);

    // 14. If targetDesc is undefined, then return true. 
    if (!targetDesc)
        return EJS_TRUE;

    // 15. If targetDesc.[[Configurable]] is false, then throw a TypeError exception. 
    // 16. Return true. 
    return EJS_TRUE;
}

static EJSBool
_ejs_proxy_specop_define_own_property (ejsval O, ejsval P, EJSPropertyDesc* Desc, EJSBool unusedflag)
{
    // 1. Assert: IsPropertyKey(P) is true. 
    EJSProxy* proxy = EJSVAL_TO_PROXY(O);

    // 2. Let handler be the value of the [[ProxyHandler]] internal slot of O. 
    ejsval handler = proxy->handler;

    // 3. If handler is null, then throw a TypeError exception. 
    if (EJSVAL_IS_NULL(handler)) {
        _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "1"); // XXX
    }

    // 4. Let target be the value of the [[ProxyTarget]] internal slot of O. 
    ejsval target = proxy->target;

    EJSObject* _target = EJSVAL_TO_OBJECT(target);

    // 5. Let trap be GetMethod(handler, "defineProperty"). 
    // 6. ReturnIfAbrupt(trap). 
    ejsval trap = GetMethod(handler, _ejs_atom_defineProperty);

    // 7. If trap is undefined, then 
    if (EJSVAL_IS_UNDEFINED(trap)) {
        //    a. Return the result of calling the [[DefineOwnProperty]] internal method of target with arguments P and Desc. 
        return _ejs_specop_define_own_property(target, P, Desc, unusedflag);
    }

    // 8. Let descObj be FromPropertyDescriptor(Desc). 
    // 9. NOTE If Desc was originally generated from an object using ToPropertyDescriptor, then descObj will be that original object. 
    ejsval descObj = FromPropertyDescriptor(Desc);

    // 10. Let trapResult be the result of calling the [[Call]] internal method of trap with handler as the this value and a new List containing target, P, and descObj. 
    ejsval args[] = { target, P, descObj };
    ejsval trapResult = _ejs_invoke_closure(trap, handler, 3, args);

    // 11. Let booleanTrapResult be ToBoolean(trapResult). 
    // 12. ReturnIfAbrupt(booleanTrapResult). 
    EJSBool booleanTrapResult = ToEJSBool(trapResult);

    // 13. If booleanTrapResult is false, then return false. 
    if (!booleanTrapResult)
        return EJS_FALSE;

    // 14. Let targetDesc be the result of calling the [[GetOwnProperty]] internal method of target with argument P. 
    // 15. ReturnIfAbrupt(targetDesc). 
    EJSPropertyDesc* targetDesc = _ejs_specop_get_own_property(target, P, NULL);

    // 16. Let extensibleTarget be IsExtensible(target). 
    // 17. ReturnIfAbrupt(extensibleTarget). 
    EJSBool extensibleTarget = EJS_OBJECT_IS_EXTENSIBLE(_target);

    EJSBool settingConfigFalse;

    // 18. If Desc has a [[Configurable]] field and if Desc.[[Configurable]] is false, then 
    if (_ejs_property_desc_has_configurable (Desc) && !_ejs_property_desc_is_configurable(Desc)) {
        //     a. Let settingConfigFalse be true. 
        settingConfigFalse = EJS_TRUE;
    }
    // 19. Else let settingConfigFalse be false. 
    else {
        settingConfigFalse = EJS_FALSE;
    }

    // 20. If targetDesc is undefined, then 
    if (!targetDesc) {
        //     a. If extensibleTarget is false, then throw a TypeError exception. 
        if (!extensibleTarget)
            _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "2"); // XXX
        //     b. If settingConfigFalse is true, then throw a TypeError exception. 
        if (settingConfigFalse)
            _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "3"); // XXX
    }
    // 21. Else targetDesc is not undefined,
    else {
        //     a. If IsCompatiblePropertyDescriptor(extensibleTarget, Desc , targetDesc) is false, then throw a TypeError exception. 
        //     b. If settingConfigFalse is true and targetDesc.[[Configurable]] is true, then throw a TypeError exception. 
        EJS_NOT_IMPLEMENTED();
    }
    // 22. Return true. 
    return EJS_TRUE;
}

static EJSObject*
_ejs_proxy_specop_allocate ()
{
    return (EJSObject*)_ejs_gc_new (EJSProxy);
}

static void
_ejs_proxy_specop_scan (EJSObject* obj, EJSValueFunc scan_func)
{
    EJSProxy* proxy = (EJSProxy*)obj;
    scan_func(proxy->target);
    scan_func(proxy->handler);
    _ejs_Object_specops.Scan (obj, scan_func);
}

EJS_DEFINE_CLASS(Proxy,
                 _ejs_proxy_specop_get_prototype_of,
                 _ejs_proxy_specop_set_prototype_of,
                 OP_INHERIT, // XXX [[IsExtensible]]
                 OP_INHERIT, // XXX [[PreventExtensions]]
                 _ejs_proxy_specop_get_own_property,
                 _ejs_proxy_specop_define_own_property,
                 _ejs_proxy_specop_has_property,
                 _ejs_proxy_specop_get,
                 _ejs_proxy_specop_set,
                 _ejs_proxy_specop_delete,
                 OP_INHERIT, // XXX [[Enumerate]]
                 OP_INHERIT, // XXX [[OwnPropertyKeys]]
                 _ejs_proxy_specop_allocate,
                 OP_INHERIT, // [[Finalize]]
                 _ejs_proxy_specop_scan
                 )


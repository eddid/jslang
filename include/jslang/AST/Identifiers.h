#define JSC_COMMON_IDENTIFIERS_EACH_PROPERTY_NAME(macro) \
    macro(__defineGetter__) \
    macro(__defineSetter__) \
    macro(__lookupGetter__) \
    macro(__lookupSetter__) \
    macro(apply) \
    macro(arguments) \
    macro(call) \
    macro(callee) \
    macro(caller) \
    macro(compile) \
    macro(configurable) \
    macro(constructor) \
    macro(create) \
    macro(defineProperty) \
    macro(defineProperties) \
    macro(enumerable) \
    macro(eval) \
    macro(exec) \
    macro(fromCharCode) \
    macro(global) \
    macro(get) \
    macro(getPrototypeOf) \
    macro(getOwnPropertyDescriptor) \
    macro(hasOwnProperty) \
    macro(ignoreCase) \
    macro(index) \
    macro(input) \
    macro(isArray) \
    macro(isPrototypeOf) \
    macro(keys) \
    macro(length) \
    macro(message) \
    macro(multiline) \
    macro(name) \
    macro(now) \
    macro(parse) \
    macro(propertyIsEnumerable) \
    macro(prototype) \
    macro(set) \
    macro(source) \
    macro(test) \
    macro(toExponential) \
    macro(toFixed) \
    macro(toISOString) \
    macro(toJSON) \
    macro(toLocaleString) \
    macro(toPrecision) \
    macro(toString) \
    macro(UTC) \
    macro(value) \
    macro(valueOf) \
    macro(writable) \
    macro(displayName)

class Identifers
{
#define IDENTIFIER_DECLARE_PROPERTY_NAME(name) const Identifier name;
#define IDENTIFIER_INITIALIZE_PROPERTY_NAME(name) , name(#name)
public:
    const Identifier nullIdentifier;
    const Identifier emptyIdentifier;
    const Identifier underscoreProto;
    const Identifier thisIdentifier;
    JSC_COMMON_IDENTIFIERS_EACH_PROPERTY_NAME(IDENTIFIER_DECLARE_PROPERTY_NAME);

public:
    Identifers()
    : nullIdentifier("")
    , emptyIdentifier("")
    , underscoreProto("__proto__")
    , thisIdentifier("this")
    JSC_COMMON_IDENTIFIERS_EACH_PROPERTY_NAME(IDENTIFIER_INITIALIZE_PROPERTY_NAME)
    {
    }
};

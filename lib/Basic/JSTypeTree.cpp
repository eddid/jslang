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

#include "JSTypeTree.h"
#include "JSObjects.h"
#include "CodeGen.h"

TBNode<JSTypeInfo> *JSTypeTree::jsObjectInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsBooleanObjectInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsNumberObjectInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsStringObjectInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsArrayObjectInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsFunctionObjectInfo = NULL;

TBNode<JSTypeInfo> *JSTypeTree::jsObjectTypeInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsBooleanTypeInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsNumberTypeInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsStringTypeInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsArrayTypeInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsFunctionTypeInfo = NULL;

TBNode<JSTypeInfo> *JSTypeTree::jsObjectProtoTypeInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsBooleanProtoTypeInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsNumberProtoTypeInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsStringProtoTypeInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsArrayProtoTypeInfo = NULL;
TBNode<JSTypeInfo> *JSTypeTree::jsFunctionProtoTypeInfo = NULL;

TBTree<JSTypeInfo> *JSTypeTree::jsTypeTree = NULL;

/**
 * when insert a new node to btree, it will call JSTypeInfoCmp to check where to insert
 * if the base of node_new is NULL, the node_new should be node_root if node_root is NULL, or should be right leaf of node_root
 * if find the node_base as the base of node_new, the node_new should be left_son of node_base.
 * but if left_son is already existed, the node_new should be right leaf of left_son
 * in another word: the left to store brothers, the right to store offspring
 */
int JSTypeInfoCmp(const void *des, const void *src, size_t size)
{
    JSTypeInfo *desTypeInfo = (JSTypeInfo *)des;
    JSTypeInfo *srcTypeInfo = (JSTypeInfo *)src;

    if (des == src)
    {
        return 0;
    }
    else if (NULL == des)
    {
        return -1;
    }
    else if (NULL == src)
    {
        return 1;
    }

    if (desTypeInfo->baseTypeInfo != srcTypeInfo->baseTypeInfo)
    {
        /**
        * if they have diffrent base,assumpsit that:
        * the src is a fixed data
        * the des is a iterator data
        * so the src must be a offspring of des
        */
        TBNode<struct JSTypeInfo> *linealTypeInfo;
        for (linealTypeInfo = srcTypeInfo->baseTypeInfo; NULL != linealTypeInfo; linealTypeInfo = linealTypeInfo->data.baseTypeInfo)
        {
            if (&linealTypeInfo->data == desTypeInfo)
            {
                /* src is a lineal offspring of des, so it is on the right branch */
                return -1;
            }
        }
#if 0
        for (linealTypeInfo = desTypeInfo->baseTypeInfo; NULL != linealTypeInfo; linealTypeInfo = linealTypeInfo->baseTypeInfo)
        {
            if (linealTypeInfo == srcTypeInfo)
            {
                /* des is a lineal offspring of src, so it is on the right branch */
                return 1;
            }
        }
        abort();
#endif
        /* src may be des's brothers's offspring */
        return 1;
    }


    if (desTypeInfo->typePtr != srcTypeInfo->typePtr)
    {
        /* same base type, but diffrent type: they are brothers, left branch */
        return 1;
    }

    if (desTypeInfo->memberName == srcTypeInfo->memberName)
    {
        return 0;
    }
    else if (NULL == desTypeInfo->memberName)
    {
        return -1;
    }
    else if (NULL == srcTypeInfo->memberName)
    {
        return 1;
    }

    return strcmp(desTypeInfo->memberName, srcTypeInfo->memberName);
}

void JSTypeInfoInit(void *ptr)
{
    if (NULL == ptr)
    {
        return ;
    }

    memset(ptr, 0x0, sizeof(JSTypeInfo));
}

void JSTypeInfoFree(void *ptr)
{
    JSTypeInfo *itemPtr = (JSTypeInfo *)ptr;

    if (NULL == ptr)
    {
        return ;
    }

    if (NULL != itemPtr->memberName)
    {
        free(itemPtr->memberName);
        itemPtr->memberName = NULL;
    }
}

void *JSTypeInfoCpy(void *to, const void *from, size_t size)
{
    JSTypeInfo *toTypeInfo = (JSTypeInfo *)to;
    JSTypeInfo *fromTypeInfo = (JSTypeInfo *)from;

    if ((to == from) || (NULL == to) || (NULL == from))
    {
        return to;
    }

    if (toTypeInfo->memberName != fromTypeInfo->memberName)
    {
        JSTypeInfoFree(to);
    }

    memcpy(to, from, sizeof(JSTypeInfo));
    if (NULL != toTypeInfo->memberName)
    {
        int length = strlen(fromTypeInfo->memberName) + 1;
        toTypeInfo->memberName = (char *)malloc(length);
        if (NULL != toTypeInfo->memberName)
        {
            memcpy((void *)toTypeInfo->memberName, (const void *)fromTypeInfo->memberName, length);
        }
    }

    return to;
}

void JSTypeInfoPrint(FILE *where, void *data)
{
    JSTypeInfo *typeInfo = (JSTypeInfo *)data;
    if ((NULL == where) || (NULL == data))
    {
        return ;
    }

    std::string name = typeInfo->typePtr->getStructName();
    std::string basename = (NULL != typeInfo->baseTypeInfo) ? typeInfo->baseTypeInfo->data.typePtr->getStructName() : "nobase";
    std::string membername = (NULL != typeInfo->memberName) ? typeInfo->memberName : "unknownmember";
    fprintf(where, "%s=%s+%s(%dB)", name.c_str(), basename.c_str(), membername.c_str(), typeInfo->memberSize);
}

void JSTypeTree::Init(CodeGenContext& context)
{
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    TBNode<JSTypeInfo> *jsObject_01Info;
    TBNode<JSTypeInfo> *jsFunctionObject_02Info;

    // store EJSObject as root
    jsTypeTree = new TBTree<JSTypeInfo>(JSTypeInfoCmp, JSTypeInfoCpy, JSTypeInfoInit, JSTypeInfoFree, JSTypeInfoPrint);
    //add all struct of Object types
    jsObject_01Info = Add(context.getTypeRef("struct.JSObject_01"), NULL, NULL, "__proto__", sizeof(struct JSObject_01));
    jsObjectInfo = Add(context.getTypeRef("struct.JSObject"), jsObject_01Info, NULL, "length", sizeof(struct EJSObject) - sizeof(struct JSObject_01));
    jsBooleanObjectInfo = jsObjectInfo;
    jsNumberObjectInfo = jsObjectInfo;
    jsStringObjectInfo = Add(context.getTypeRef("struct.JSString"), jsObjectInfo, NULL, NULL, sizeof(struct JSString) - sizeof(struct EJSObject));
    jsArrayObjectInfo = Add(context.getTypeRef("struct.JSArrayObject"), jsObjectInfo, NULL, NULL, sizeof(struct EJSArray) - sizeof(struct EJSObject));
    jsFunctionObjectInfo = Add(context.getTypeRef("struct.EJSFunctionObject_01"), jsObjectInfo, jsArrayObjectInfo, "arguments", sizeof(struct EJSFunctionObject_01) - sizeof(struct EJSObject));
    jsFunctionObject_02Info = Add(context.getTypeRef("struct.EJSFunctionObject_02"), jsFunctionObjectInfo, NULL, "caller", sizeof(struct EJSFunctionObject_02) - sizeof(struct EJSFunctionObject_01));
    jsFunctionObjectInfo = Add(context.getTypeRef("struct.EJSFunctionObject_03"), jsFunctionObject_02Info, jsStringObjectInfo, "displayName", sizeof(struct EJSFunctionObject_03) - sizeof(struct EJSFunctionObject_02));
    jsFunctionObjectInfo = Add(context.getTypeRef("struct.EJSFunction"), jsFunctionObjectInfo, jsStringObjectInfo, "name", sizeof(struct EJSFunction) - sizeof(struct EJSFunctionObject_03));
    
    //add all struct of Type types
    jsFunctionTypeInfo = Add(context.getTypeRef("struct.EJSFunctionType"), jsFunctionObjectInfo, NULL, "prototype", sizeof(struct EJSFunctionType) - sizeof(struct EJSFunction));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_01"), jsFunctionTypeInfo, NULL, "create", sizeof(struct JSObjectType_01) - sizeof(struct EJSFunctionType));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_02"), jsObjectTypeInfo, NULL, "defineProperty", sizeof(struct JSObjectType_02) - sizeof(struct JSObjectType_01));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_03"), jsObjectTypeInfo, NULL, "defineProperties", sizeof(struct JSObjectType_03) - sizeof(struct JSObjectType_02));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_04"), jsObjectTypeInfo, NULL, "freeze", sizeof(struct JSObjectType_04) - sizeof(struct JSObjectType_03));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_05"), jsObjectTypeInfo, NULL, "getOwnPropertyDescriptor", sizeof(struct JSObjectType_05) - sizeof(struct JSObjectType_04));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_06"), jsObjectTypeInfo, NULL, "getOwnPropertyNames", sizeof(struct JSObjectType_06) - sizeof(struct JSObjectType_05));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_07"), jsObjectTypeInfo, NULL, "getOwnPropertySymbols", sizeof(struct JSObjectType_07) - sizeof(struct JSObjectType_06));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_08"), jsObjectTypeInfo, NULL, "getPrototypeOf", sizeof(struct JSObjectType_08) - sizeof(struct JSObjectType_07));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_09"), jsObjectTypeInfo, NULL, "is", sizeof(struct JSObjectType_09) - sizeof(struct JSObjectType_08));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_10"), jsObjectTypeInfo, NULL, "isExtensible", sizeof(struct JSObjectType_10) - sizeof(struct JSObjectType_09));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_11"), jsObjectTypeInfo, NULL, "isFrozen", sizeof(struct JSObjectType_11) - sizeof(struct JSObjectType_10));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_12"), jsObjectTypeInfo, NULL, "isSealed", sizeof(struct JSObjectType_12) - sizeof(struct JSObjectType_11));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_13"), jsObjectTypeInfo, NULL, "keys", sizeof(struct JSObjectType_13) - sizeof(struct JSObjectType_12));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_14"), jsObjectTypeInfo, NULL, "preventExtensions", sizeof(struct JSObjectType_14) - sizeof(struct JSObjectType_13));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType_15"), jsObjectTypeInfo, NULL, "seal", sizeof(struct JSObjectType_15) - sizeof(struct JSObjectType_14));
    jsObjectTypeInfo = Add(context.getTypeRef("struct.JSObjectType"), jsObjectTypeInfo, NULL, "setPrototypeOf", sizeof(struct JSObjectType) - sizeof(struct JSObjectType_15));
    jsBooleanTypeInfo = jsFunctionTypeInfo;
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_01"), jsFunctionTypeInfo, NULL, "EPSILON", sizeof(struct JSNumberType_01) - sizeof(struct EJSFunctionType));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_02"), jsNumberTypeInfo, NULL, "MAX_SAFE_INTEGER", sizeof(struct JSNumberType_02) - sizeof(struct JSNumberType_01));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_03"), jsNumberTypeInfo, NULL, "MIN_SAFE_INTEGER", sizeof(struct JSNumberType_03) - sizeof(struct JSNumberType_02));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_04"), jsNumberTypeInfo, NULL, "MAX_VALUE", sizeof(struct JSNumberType_04) - sizeof(struct JSNumberType_03));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_05"), jsNumberTypeInfo, NULL, "MIN_VALUE", sizeof(struct JSNumberType_05) - sizeof(struct JSNumberType_04));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_06"), jsNumberTypeInfo, NULL, "NaN", sizeof(struct JSNumberType_06) - sizeof(struct JSNumberType_05));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_07"), jsNumberTypeInfo, NULL, "NEGATIVE_INFINITY", sizeof(struct JSNumberType_07) - sizeof(struct JSNumberType_06));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_08"), jsNumberTypeInfo, NULL, "POSITIVE_INFINITY", sizeof(struct JSNumberType_08) - sizeof(struct JSNumberType_07));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_09"), jsNumberTypeInfo, NULL, "isFinite", sizeof(struct JSNumberType_09) - sizeof(struct JSNumberType_08));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_10"), jsNumberTypeInfo, NULL, "isInteger", sizeof(struct JSNumberType_10) - sizeof(struct JSNumberType_09));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_11"), jsNumberTypeInfo, NULL, "isNaN", sizeof(struct JSNumberType_11) - sizeof(struct JSNumberType_10));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_12"), jsNumberTypeInfo, NULL, "isSafeInteger", sizeof(struct JSNumberType_12) - sizeof(struct JSNumberType_11));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType_13"), jsNumberTypeInfo, NULL, "parseFloat", sizeof(struct JSNumberType_13) - sizeof(struct JSNumberType_12));
    jsNumberTypeInfo = Add(context.getTypeRef("struct.JSNumberType"), jsNumberTypeInfo, NULL, "parseInt", sizeof(struct JSNumberType) - sizeof(struct JSNumberType_13));
    jsStringTypeInfo = Add(context.getTypeRef("struct.JSStringType_01"), jsFunctionTypeInfo, NULL, "fromCharCode", sizeof(struct JSStringType_01) - sizeof(struct EJSFunctionType));
    jsStringTypeInfo = Add(context.getTypeRef("struct.JSStringType"), jsStringTypeInfo, NULL, "fromCodePoint", sizeof(struct JSStringType) - sizeof(struct JSStringType_01));
    jsArrayTypeInfo = Add(context.getTypeRef("struct.JSArrayType_01"), jsFunctionTypeInfo, NULL, "from", sizeof(struct JSArrayType_01) - sizeof(struct EJSFunctionType));
    jsArrayTypeInfo = Add(context.getTypeRef("struct.JSArrayType_02"), jsArrayTypeInfo, NULL, "isArray", sizeof(struct JSArrayType_02) - sizeof(struct JSArrayType_01));
    jsArrayTypeInfo = Add(context.getTypeRef("struct.JSArrayType"), jsArrayTypeInfo, NULL, "of", sizeof(struct JSArrayType) - sizeof(struct JSArrayType_02));

    //add all struct of Prototype types
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_01"), jsObjectInfo, NULL, "__defineGetter__", sizeof(struct JSPrototype_01) - sizeof(struct EJSObject));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_02"), jsObjectProtoTypeInfo, NULL, "__defineSetter__", sizeof(struct JSPrototype_02) - sizeof(struct JSPrototype_01));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_03"), jsObjectProtoTypeInfo, NULL, "__lookupGetter__", sizeof(struct JSPrototype_03) - sizeof(struct JSPrototype_02));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_04"), jsObjectProtoTypeInfo, NULL, "__lookupSetter__", sizeof(struct JSPrototype_04) - sizeof(struct JSPrototype_03));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_05"), jsObjectProtoTypeInfo, NULL, "hasOwnProperty", sizeof(struct JSPrototype_05) - sizeof(struct JSPrototype_04));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_06"), jsObjectProtoTypeInfo, NULL, "isPrototypeOf", sizeof(struct JSPrototype_06) - sizeof(struct JSPrototype_05));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_07"), jsObjectProtoTypeInfo, NULL, "propertyIsEnumerable", sizeof(struct JSPrototype_07) - sizeof(struct JSPrototype_06));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_08"), jsObjectProtoTypeInfo, NULL, "toLocaleString", sizeof(struct JSPrototype_08) - sizeof(struct JSPrototype_07));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_09"), jsObjectProtoTypeInfo, NULL, "toSource", sizeof(struct JSPrototype_09) - sizeof(struct JSPrototype_08));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_10"), jsObjectProtoTypeInfo, NULL, "toString", sizeof(struct JSPrototype_10) - sizeof(struct JSPrototype_09));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_11"), jsObjectProtoTypeInfo, NULL, "unwatch", sizeof(struct JSPrototype_11) - sizeof(struct JSPrototype_10));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_12"), jsObjectProtoTypeInfo, NULL, "valueOf", sizeof(struct JSPrototype_12) - sizeof(struct JSPrototype_11));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_13"), jsObjectProtoTypeInfo, NULL, "watch", sizeof(struct JSPrototype_13) - sizeof(struct JSPrototype_12));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype_14"), jsObjectProtoTypeInfo, NULL, "constructor", sizeof(struct JSPrototype_14) - sizeof(struct JSPrototype_13));
    jsObjectProtoTypeInfo = Add(context.getTypeRef("struct.JSPrototype"), jsObjectProtoTypeInfo, NULL, "__noSuchMethod__", sizeof(struct JSPrototype) - sizeof(struct JSPrototype_14));
    jsBooleanProtoTypeInfo = Add(context.getTypeRef("struct.JSBooleanPrototype_01"), jsObjectInfo, NULL, "toSource", sizeof(struct JSBooleanPrototype_01) - sizeof(struct EJSObject));
    jsBooleanProtoTypeInfo = Add(context.getTypeRef("struct.JSBooleanPrototype_02"), jsBooleanProtoTypeInfo, NULL, "toString", sizeof(struct JSBooleanPrototype_02) - sizeof(struct JSBooleanPrototype_01));
    jsBooleanProtoTypeInfo = Add(context.getTypeRef("struct.JSBooleanPrototype"), jsBooleanProtoTypeInfo, NULL, "valueOf", sizeof(struct JSBooleanPrototype) - sizeof(struct JSBooleanPrototype_02));
    jsNumberProtoTypeInfo = Add(context.getTypeRef("struct.JSNumberPrototype_01"), jsObjectInfo, NULL, "toExponential", sizeof(struct JSNumberPrototype_01) - sizeof(struct EJSObject));
    jsNumberProtoTypeInfo = Add(context.getTypeRef("struct.JSNumberPrototype_02"), jsNumberProtoTypeInfo, NULL, "toFixed", sizeof(struct JSNumberPrototype_02) - sizeof(struct JSNumberPrototype_01));
    jsNumberProtoTypeInfo = Add(context.getTypeRef("struct.JSNumberPrototype_03"), jsNumberProtoTypeInfo, NULL, "toLocaleString", sizeof(struct JSNumberPrototype_03) - sizeof(struct JSNumberPrototype_02));
    jsNumberProtoTypeInfo = Add(context.getTypeRef("struct.JSNumberPrototype_04"), jsNumberProtoTypeInfo, NULL, "toPrecision", sizeof(struct JSNumberPrototype_04) - sizeof(struct JSNumberPrototype_03));
    jsNumberProtoTypeInfo = Add(context.getTypeRef("struct.JSNumberPrototype_05"), jsNumberProtoTypeInfo, NULL, "toSource", sizeof(struct JSNumberPrototype_05) - sizeof(struct JSNumberPrototype_04));
    jsNumberProtoTypeInfo = Add(context.getTypeRef("struct.JSNumberPrototype_06"), jsNumberProtoTypeInfo, NULL, "toString", sizeof(struct JSNumberPrototype_06) - sizeof(struct JSNumberPrototype_05));
    jsNumberProtoTypeInfo = Add(context.getTypeRef("struct.JSNumberPrototype"), jsNumberProtoTypeInfo, NULL, "valueOf", sizeof(struct JSNumberPrototype) - sizeof(struct JSNumberPrototype_06));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_01"), jsObjectInfo, NULL, "anchor", sizeof(struct JSStringPrototype_01) - sizeof(struct EJSObject));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_02"), jsStringProtoTypeInfo, NULL, "big", sizeof(struct JSStringPrototype_02) - sizeof(struct JSStringPrototype_01));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_03"), jsStringProtoTypeInfo, NULL, "blink", sizeof(struct JSStringPrototype_03) - sizeof(struct JSStringPrototype_02));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_04"), jsStringProtoTypeInfo, NULL, "bold", sizeof(struct JSStringPrototype_04) - sizeof(struct JSStringPrototype_03));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_05"), jsStringProtoTypeInfo, NULL, "charAt", sizeof(struct JSStringPrototype_05) - sizeof(struct JSStringPrototype_04));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_06"), jsStringProtoTypeInfo, NULL, "charCodeAt", sizeof(struct JSStringPrototype_06) - sizeof(struct JSStringPrototype_05));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_07"), jsStringProtoTypeInfo, NULL, "codePointAt", sizeof(struct JSStringPrototype_07) - sizeof(struct JSStringPrototype_06));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_08"), jsStringProtoTypeInfo, NULL, "concat", sizeof(struct JSStringPrototype_08) - sizeof(struct JSStringPrototype_07));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_09"), jsStringProtoTypeInfo, NULL, "contains", sizeof(struct JSStringPrototype_09) - sizeof(struct JSStringPrototype_08));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_10"), jsStringProtoTypeInfo, NULL, "endsWith", sizeof(struct JSStringPrototype_10) - sizeof(struct JSStringPrototype_09));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_11"), jsStringProtoTypeInfo, NULL, "fixed", sizeof(struct JSStringPrototype_11) - sizeof(struct JSStringPrototype_10));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_12"), jsStringProtoTypeInfo, NULL, "fontcolor", sizeof(struct JSStringPrototype_12) - sizeof(struct JSStringPrototype_11));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_13"), jsStringProtoTypeInfo, NULL, "fontsize", sizeof(struct JSStringPrototype_13) - sizeof(struct JSStringPrototype_12));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_14"), jsStringProtoTypeInfo, NULL, "indexOf", sizeof(struct JSStringPrototype_14) - sizeof(struct JSStringPrototype_13));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_15"), jsStringProtoTypeInfo, NULL, "italics", sizeof(struct JSStringPrototype_15) - sizeof(struct JSStringPrototype_14));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_16"), jsStringProtoTypeInfo, NULL, "lastIndexOf", sizeof(struct JSStringPrototype_16) - sizeof(struct JSStringPrototype_15));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_17"), jsStringProtoTypeInfo, NULL, "link", sizeof(struct JSStringPrototype_17) - sizeof(struct JSStringPrototype_16));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_18"), jsStringProtoTypeInfo, NULL, "localeCompare", sizeof(struct JSStringPrototype_18) - sizeof(struct JSStringPrototype_17));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_19"), jsStringProtoTypeInfo, NULL, "match", sizeof(struct JSStringPrototype_19) - sizeof(struct JSStringPrototype_18));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_20"), jsStringProtoTypeInfo, NULL, "normalize", sizeof(struct JSStringPrototype_20) - sizeof(struct JSStringPrototype_19));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_21"), jsStringProtoTypeInfo, NULL, "quote", sizeof(struct JSStringPrototype_21) - sizeof(struct JSStringPrototype_20));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_22"), jsStringProtoTypeInfo, NULL, "repeat", sizeof(struct JSStringPrototype_22) - sizeof(struct JSStringPrototype_21));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_23"), jsStringProtoTypeInfo, NULL, "replace", sizeof(struct JSStringPrototype_23) - sizeof(struct JSStringPrototype_22));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_24"), jsStringProtoTypeInfo, NULL, "search", sizeof(struct JSStringPrototype_24) - sizeof(struct JSStringPrototype_23));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_25"), jsStringProtoTypeInfo, NULL, "slice", sizeof(struct JSStringPrototype_25) - sizeof(struct JSStringPrototype_24));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_26"), jsStringProtoTypeInfo, NULL, "small", sizeof(struct JSStringPrototype_26) - sizeof(struct JSStringPrototype_25));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_27"), jsStringProtoTypeInfo, NULL, "split", sizeof(struct JSStringPrototype_27) - sizeof(struct JSStringPrototype_26));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_28"), jsStringProtoTypeInfo, NULL, "startsWith", sizeof(struct JSStringPrototype_28) - sizeof(struct JSStringPrototype_27));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_29"), jsStringProtoTypeInfo, NULL, "strike", sizeof(struct JSStringPrototype_29) - sizeof(struct JSStringPrototype_28));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_30"), jsStringProtoTypeInfo, NULL, "sub", sizeof(struct JSStringPrototype_30) - sizeof(struct JSStringPrototype_29));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_31"), jsStringProtoTypeInfo, NULL, "substr", sizeof(struct JSStringPrototype_31) - sizeof(struct JSStringPrototype_30));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_32"), jsStringProtoTypeInfo, NULL, "substring", sizeof(struct JSStringPrototype_32) - sizeof(struct JSStringPrototype_31));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_33"), jsStringProtoTypeInfo, NULL, "sup", sizeof(struct JSStringPrototype_33) - sizeof(struct JSStringPrototype_32));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_34"), jsStringProtoTypeInfo, NULL, "toLocaleLowerCase", sizeof(struct JSStringPrototype_34) - sizeof(struct JSStringPrototype_33));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_35"), jsStringProtoTypeInfo, NULL, "toLocaleUpperCase", sizeof(struct JSStringPrototype_35) - sizeof(struct JSStringPrototype_34));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_36"), jsStringProtoTypeInfo, NULL, "toLowerCase", sizeof(struct JSStringPrototype_36) - sizeof(struct JSStringPrototype_35));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_37"), jsStringProtoTypeInfo, NULL, "toSource", sizeof(struct JSStringPrototype_37) - sizeof(struct JSStringPrototype_36));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_38"), jsStringProtoTypeInfo, NULL, "toString", sizeof(struct JSStringPrototype_38) - sizeof(struct JSStringPrototype_37));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_39"), jsStringProtoTypeInfo, NULL, "toUpperCase", sizeof(struct JSStringPrototype_39) - sizeof(struct JSStringPrototype_38));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_40"), jsStringProtoTypeInfo, NULL, "trim", sizeof(struct JSStringPrototype_40) - sizeof(struct JSStringPrototype_39));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_41"), jsStringProtoTypeInfo, NULL, "trimLeft", sizeof(struct JSStringPrototype_41) - sizeof(struct JSStringPrototype_40));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype_42"), jsStringProtoTypeInfo, NULL, "trimRight", sizeof(struct JSStringPrototype_42) - sizeof(struct JSStringPrototype_41));
    jsStringProtoTypeInfo = Add(context.getTypeRef("struct.JSStringPrototype"), jsStringProtoTypeInfo, NULL, "valueOf", sizeof(struct JSStringPrototype) - sizeof(struct JSStringPrototype_42));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_01"), jsObjectInfo, NULL, "concat", sizeof(struct JSArrayPrototype_01) - sizeof(struct EJSObject));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_02"), jsArrayProtoTypeInfo, NULL, "copyWithin", sizeof(struct JSArrayPrototype_02) - sizeof(struct JSArrayPrototype_01));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_03"), jsArrayProtoTypeInfo, NULL, "entries", sizeof(struct JSArrayPrototype_03) - sizeof(struct JSArrayPrototype_02));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_04"), jsArrayProtoTypeInfo, NULL, "every", sizeof(struct JSArrayPrototype_04) - sizeof(struct JSArrayPrototype_03));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_05"), jsArrayProtoTypeInfo, NULL, "fill", sizeof(struct JSArrayPrototype_05) - sizeof(struct JSArrayPrototype_04));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_06"), jsArrayProtoTypeInfo, NULL, "filter", sizeof(struct JSArrayPrototype_06) - sizeof(struct JSArrayPrototype_05));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_07"), jsArrayProtoTypeInfo, NULL, "find", sizeof(struct JSArrayPrototype_07) - sizeof(struct JSArrayPrototype_06));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_08"), jsArrayProtoTypeInfo, NULL, "findIndex", sizeof(struct JSArrayPrototype_08) - sizeof(struct JSArrayPrototype_07));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_09"), jsArrayProtoTypeInfo, NULL, "forEach", sizeof(struct JSArrayPrototype_09) - sizeof(struct JSArrayPrototype_08));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_10"), jsArrayProtoTypeInfo, NULL, "indexOf", sizeof(struct JSArrayPrototype_10) - sizeof(struct JSArrayPrototype_09));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_11"), jsArrayProtoTypeInfo, NULL, "join", sizeof(struct JSArrayPrototype_11) - sizeof(struct JSArrayPrototype_10));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_12"), jsArrayProtoTypeInfo, NULL, "keys", sizeof(struct JSArrayPrototype_12) - sizeof(struct JSArrayPrototype_11));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_13"), jsArrayProtoTypeInfo, NULL, "lastIndexOf", sizeof(struct JSArrayPrototype_13) - sizeof(struct JSArrayPrototype_12));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_14"), jsArrayProtoTypeInfo, NULL, "map", sizeof(struct JSArrayPrototype_14) - sizeof(struct JSArrayPrototype_13));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_15"), jsArrayProtoTypeInfo, NULL, "pop", sizeof(struct JSArrayPrototype_15) - sizeof(struct JSArrayPrototype_14));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_16"), jsArrayProtoTypeInfo, NULL, "push", sizeof(struct JSArrayPrototype_16) - sizeof(struct JSArrayPrototype_15));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_17"), jsArrayProtoTypeInfo, NULL, "reduce", sizeof(struct JSArrayPrototype_17) - sizeof(struct JSArrayPrototype_16));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_18"), jsArrayProtoTypeInfo, NULL, "reduceRight", sizeof(struct JSArrayPrototype_18) - sizeof(struct JSArrayPrototype_17));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_19"), jsArrayProtoTypeInfo, NULL, "reverse", sizeof(struct JSArrayPrototype_19) - sizeof(struct JSArrayPrototype_18));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_20"), jsArrayProtoTypeInfo, NULL, "shift", sizeof(struct JSArrayPrototype_20) - sizeof(struct JSArrayPrototype_19));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_21"), jsArrayProtoTypeInfo, NULL, "slice", sizeof(struct JSArrayPrototype_21) - sizeof(struct JSArrayPrototype_20));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_22"), jsArrayProtoTypeInfo, NULL, "some", sizeof(struct JSArrayPrototype_22) - sizeof(struct JSArrayPrototype_21));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_23"), jsArrayProtoTypeInfo, NULL, "sort", sizeof(struct JSArrayPrototype_23) - sizeof(struct JSArrayPrototype_22));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_24"), jsArrayProtoTypeInfo, NULL, "splice", sizeof(struct JSArrayPrototype_24) - sizeof(struct JSArrayPrototype_23));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_25"), jsArrayProtoTypeInfo, NULL, "toLocaleString", sizeof(struct JSArrayPrototype_25) - sizeof(struct JSArrayPrototype_24));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_26"), jsArrayProtoTypeInfo, NULL, "toSource", sizeof(struct JSArrayPrototype_26) - sizeof(struct JSArrayPrototype_25));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype_27"), jsArrayProtoTypeInfo, NULL, "toString", sizeof(struct JSArrayPrototype_27) - sizeof(struct JSArrayPrototype_26));
    jsArrayProtoTypeInfo = Add(context.getTypeRef("struct.JSArrayPrototype"), jsArrayProtoTypeInfo, NULL, "unshift", sizeof(struct JSArrayPrototype) - sizeof(struct JSArrayPrototype_27));
    jsFunctionProtoTypeInfo = Add(context.getTypeRef("struct.EJSFunctionPrototype_01"), jsObjectInfo, NULL, "apply", sizeof(struct EJSFunctionPrototype_01) - sizeof(struct EJSObject));
    jsFunctionProtoTypeInfo = Add(context.getTypeRef("struct.EJSFunctionPrototype_02"), jsFunctionProtoTypeInfo, NULL, "bind", sizeof(struct EJSFunctionPrototype_02) - sizeof(struct EJSFunctionPrototype_01));
    jsFunctionProtoTypeInfo = Add(context.getTypeRef("struct.EJSFunctionPrototype_03"), jsFunctionProtoTypeInfo, NULL, "call", sizeof(struct EJSFunctionPrototype_03) - sizeof(struct EJSFunctionPrototype_02));
    jsFunctionProtoTypeInfo = Add(context.getTypeRef("struct.EJSFunctionPrototype_04"), jsFunctionProtoTypeInfo, NULL, "isGenerator", sizeof(struct EJSFunctionPrototype_04) - sizeof(struct EJSFunctionPrototype_03));
    jsFunctionProtoTypeInfo = Add(context.getTypeRef("struct.EJSFunctionPrototype_05"), jsFunctionProtoTypeInfo, NULL, "toSource", sizeof(struct EJSFunctionPrototype_05) - sizeof(struct EJSFunctionPrototype_04));
    jsFunctionProtoTypeInfo = Add(context.getTypeRef("struct.EJSFunctionPrototype"), jsFunctionProtoTypeInfo, NULL, "toString", sizeof(struct EJSFunctionPrototype) - sizeof(struct EJSFunctionPrototype_05));

    //update type info if needed
    ((JSTypeInfo *)(&jsFunctionObject_02Info->data))->memberTypeInfo = jsFunctionObjectInfo;
    ((JSTypeInfo *)(&jsObject_01Info->data))->memberTypeInfo = jsObjectProtoTypeInfo;
    ((JSTypeInfo *)(&jsFunctionTypeInfo->data))->memberTypeInfo = jsObjectProtoTypeInfo;

    jsTypeTree->print();
}

TBNode<JSTypeInfo> *JSTypeTree::Add(llvm::Type *type, TBNode<JSTypeInfo> *base, TBNode<JSTypeInfo> *member, const char *name, int size)
{
    JSTypeInfo newTypeInfo;

    if (NULL == type)
    {
        return NULL;
    }
    
    newTypeInfo.typePtr = type;
    newTypeInfo.baseTypeInfo = base;
    newTypeInfo.memberTypeInfo = member;
    newTypeInfo.memberName = (char *)name;
    newTypeInfo.memberSize = size;

    return jsTypeTree->Insert(newTypeInfo);
}

TBNode<JSTypeInfo> *JSTypeTree::FindType(llvm::Type *type)
{
    JSTypeInfo *typeInfo;
    std::vector<TBNode<JSTypeInfo> *> nodeList;
    TBNode<JSTypeInfo> *node = jsTypeTree->mRoot;

    if (NULL == type)
    {
        return NULL;
    }

    while (NULL != node)
    {
        typeInfo = &node->data;
        if (type == typeInfo->typePtr)
        {
            break;
        }
        if (NULL != node->right)
        {
            nodeList.push_back(node->right);
        }
        if (NULL != node->left)
        {
            node = node->left;
        }
        else if (nodeList.empty())
        {
            node = NULL;
        }
        else
        {
            node = nodeList.back();
            nodeList.pop_back();
        }
    }

    return node;
}

TBNode<JSTypeInfo> *JSTypeTree::FindMemberInPrototype(CodeGenContext& context, TBNode<JSTypeInfo> *node, llvm::Type *member, const char *name, llvm::Value *nodeValueRef)
{
    JSTypeInfo *typeInfo;
    TBNode<JSTypeInfo> *nodeContainMember = node;

    if ((NULL == node) || (NULL == name))
    {
        return node;
    }

    /* find __proto__ first, two ways:JSTypeInfoFindInPredecessor+JSTypeInfoGetMemberValueOfBase, or suppose EJSObject(JSObject_01 actually) is the base of every type  */
    TBNode<JSTypeInfo> *jsObject_01Info;
#if 0
    jsObject_01Info= FindType(context.getTypeRef("struct.JSObject_01"));
#else
    jsObject_01Info= jsTypeTree->mRoot;
#endif
    llvm::Value *prototypeRef = GetMemberRef(context, node, jsObject_01Info, "__proto__", nodeValueRef);
    llvm::Value *valueRef = GetMemberRef(context, node, jsObject_01Info, "__proto__", prototypeRef);

    /* check predecessor of typeInfo to find name */
    for (nodeContainMember = node; NULL != nodeContainMember; nodeContainMember = nodeContainMember->data.baseTypeInfo)
    {
        typeInfo = &nodeContainMember->data;
        if (NULL == typeInfo->memberName)
        {
            continue;
        }
        if (0 == strcmp(typeInfo->memberName, name))
        {
            break;
        }
    }

    /* find name in the base, no need to create a new type for the property */
    return nodeContainMember;
}

TBNode<JSTypeInfo> *JSTypeTree::FindMemberInPredecessor(TBNode<JSTypeInfo> *node, llvm::Type *member, const char *name)
{
    JSTypeInfo *typeInfo;
    TBNode<JSTypeInfo> *nodeContainMember = node;

    if ((NULL == node) || (NULL == name))
    {
        return node;
    }

    /* check predecessor of typeInfo to find name */
    for (nodeContainMember = node; NULL != nodeContainMember; nodeContainMember = nodeContainMember->data.baseTypeInfo)
    {
        typeInfo = &nodeContainMember->data;
        if (NULL == typeInfo->memberName)
        {
            continue;
        }
        if (0 == strcmp(typeInfo->memberName, name))
        {
            break;
        }
    }

    /* find name in the base, no need to create a new type for the property */
    return nodeContainMember;
}

TBNode<JSTypeInfo> *JSTypeTree::FindMemberInSuccessor(TBNode<JSTypeInfo> *node, llvm::Type *member, const char *name)
{
    JSTypeInfo *typeInfo;

    if ((NULL == node) || (NULL == name))
    {
        return node;
    }

    /* try to find name in the son of typeInfo, maybe we needn't create a new type */
    TBNode<JSTypeInfo> *subNode;
    for (subNode = node->right; NULL != subNode; subNode = subNode->left)
    {
        typeInfo = &subNode->data;

        if (NULL == typeInfo->memberName)
        {
            continue;
        }
        if (0 == strcmp(typeInfo->memberName, name))
        {
            break;
        }
    }

    return subNode;
}

llvm::Value *JSTypeTree::GetMemberRef(CodeGenContext& context, TBNode<JSTypeInfo> *node, TBNode<JSTypeInfo> *nodeContainMember, const char *name, llvm::Value *nodeValueRef)
{
    if ((NULL == node) || (NULL == nodeContainMember))
    {
        return NULL;
    }
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Value *asBaseRef = nodeValueRef;
    JSTypeInfo *baseTypeInfo = &nodeContainMember->data;
    if (node != nodeContainMember)
    {
        llvm::Type *basePtrType = llvm::PointerType::get(baseTypeInfo->typePtr, 0);
        asBaseRef = llvm::CastInst::Create(llvm::Instruction::BitCast, nodeValueRef, basePtrType, "asBaseRef", context.currentBlock());
    }
    std::vector<llvm::Value*> indexList;
    indexList.push_back(llvm::ConstantInt::get(int32Type, 0));
    if (NULL == baseTypeInfo->baseTypeInfo)
    {
        indexList.push_back(llvm::ConstantInt::get(int32Type, 0));
    }
    else
    {
        indexList.push_back(llvm::ConstantInt::get(int32Type, 1));
    }
    llvm::Value *memberRef = llvm::GetElementPtrInst::Create(asBaseRef, indexList, name, context.currentBlock());
    return memberRef;
}

llvm::Value *JSTypeTree::SetMemberValue(CodeGenContext& context, TBNode<JSTypeInfo> *node, TBNode<JSTypeInfo> *nodeContainMember, const char *name, llvm::Value *nodeValueRef, llvm::Value *memberValue)
{
    if ((NULL == node) || (NULL == nodeContainMember))
    {
        return NULL;
    }
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Value *asBaseRef = nodeValueRef;
    JSTypeInfo *baseTypeInfo = &nodeContainMember->data;
    if (node != nodeContainMember)
    {
        llvm::Type *basePtrType = llvm::PointerType::get(baseTypeInfo->typePtr, 0);
        asBaseRef = llvm::CastInst::Create(llvm::Instruction::BitCast, nodeValueRef, basePtrType, "asBaseRef", context.currentBlock());
    }
    std::vector<llvm::Value*> indexList;
    indexList.push_back(llvm::ConstantInt::get(int32Type, 0));
    if (NULL == baseTypeInfo->baseTypeInfo)
    {
        indexList.push_back(llvm::ConstantInt::get(int32Type, 0));
    }
    else
    {
        indexList.push_back(llvm::ConstantInt::get(int32Type, 1));
    }
    llvm::Value *memberRef = llvm::GetElementPtrInst::Create(asBaseRef, indexList, name, context.currentBlock());
    //new llvm::StoreInst(memberValue, memberRef, false, context.currentBlock());
    CodeGenJSValue32FunctionCopy(context, memberRef, memberValue);
    
    return memberValue;
}

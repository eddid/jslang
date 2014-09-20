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

#include "NodeInfo.h"
#include "CodeGen.h"
#include <iostream>

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

using namespace std;

typedef struct
{
    const char *name;
    const char *value;
}StringVarialbePaire;

const static StringVarialbePaire stringVars[] = 
{
    {"intFormat", "%d"},
    {"doubleFormat", "%f"},
    {"trueString", "true"},
    {"falseString", "false"},
    {"nullString", "null"},
    {"undefinedString", "undefined"},
    {"objectString", "object"},
    {"doubleEnterFormat", "%f\n"},
    {"jsValueFormat", "%f,rawdata:0x%08x-%08x\n"},
    {NULL, NULL},
};
void declareVariables(CodeGenContext& context)
{
    llvm::IntegerType *int8Type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Constant *zero = llvm::Constant::getNullValue(int32Type);
    std::vector<llvm::Constant*> indices;
    
    indices.push_back(zero);
    indices.push_back(zero);

    for (int stringVarIndex = 0; NULL != stringVars[stringVarIndex].name; stringVarIndex++)
    {
        llvm::Constant *constArray = llvm::ConstantDataArray::getString(llvm::getGlobalContext(), stringVars[stringVarIndex].value, true);
        llvm::ArrayType* arrayType = llvm::ArrayType::get(int8Type, strlen(stringVars[stringVarIndex].value) + 1);
        llvm::GlobalVariable *globalVariable = new llvm::GlobalVariable(*context.getModule(), arrayType, true, llvm::GlobalValue::PrivateLinkage, constArray, stringVars[stringVarIndex].name);
        llvm::Constant *varRef = llvm::ConstantExpr::getGetElementPtr(globalVariable, indices);
        context.setVariableRef(stringVars[stringVarIndex].name, varRef, false);
    }
}

void declareFunctions(CodeGenContext& context)
{
    llvm::PointerType *int8PtrType = llvm::Type::getInt8PtrTy(llvm::getGlobalContext());
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    
    std::vector<llvm::Type*> argTypes;
    llvm::FunctionType* functionType = NULL;
    llvm::Function *function = NULL;

    //extern int snprintf ( char * s, size_t n, const char * format, ... );
    argTypes.push_back(int8PtrType);
    argTypes.push_back(int32Type);
    argTypes.push_back(int8PtrType);
    functionType = llvm::FunctionType::get(int32Type, argTypes, true);
    function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "snprintf", context.getModule());
    function->setCallingConv(llvm::CallingConv::C);
    argTypes.clear();

    //extern void *memcpy(void * destination, const void * source, unsigned int num);
    argTypes.push_back(int8PtrType);
    argTypes.push_back(int8PtrType);
    argTypes.push_back(int32Type);
    functionType = llvm::FunctionType::get(int8PtrType, argTypes, false);
    function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "memcpy", context.getModule());
    function->setCallingConv(llvm::CallingConv::C);
    argTypes.clear();
}

typedef struct
{
    void *external;
    char *realname;
    char *showname;
}ExternalVar;
#define EXTERNAL_FUNCTION_ITEM(f) {(void *)f, #f, NULL}
ExternalVar externalFunctions[] =
{
    EXTERNAL_FUNCTION_ITEM(jsextern_print_tick),
    EXTERNAL_FUNCTION_ITEM(jsextern_os_msleep),
    EXTERNAL_FUNCTION_ITEM(jsextern_os_swap),
    EXTERNAL_FUNCTION_ITEM(jsextern_pcre_compile),
    EXTERNAL_FUNCTION_ITEM(jsextern_pcre_study),
    EXTERNAL_FUNCTION_ITEM(jsextern_pcre_bracketcount),
    EXTERNAL_FUNCTION_ITEM(jsextern_pcre_exec),
    EXTERNAL_FUNCTION_ITEM(jsextern_pcre_free),
    EXTERNAL_FUNCTION_ITEM(jsextern_thread_create),
    EXTERNAL_FUNCTION_ITEM(jsextern_thread_destroy),
    EXTERNAL_FUNCTION_ITEM(jsextern_mutex_create),
    EXTERNAL_FUNCTION_ITEM(jsextern_mutex_destroy),
    EXTERNAL_FUNCTION_ITEM(jsextern_mutex_lock),
    EXTERNAL_FUNCTION_ITEM(jsextern_mutex_unlock),
    EXTERNAL_FUNCTION_ITEM(jsextern_signal_create),
    EXTERNAL_FUNCTION_ITEM(jsextern_signal_destroy),
    EXTERNAL_FUNCTION_ITEM(jsextern_signal_wait),
    EXTERNAL_FUNCTION_ITEM(jsextern_signal_send),
    EXTERNAL_FUNCTION_ITEM(JSValueHash),
    EXTERNAL_FUNCTION_ITEM(_ejs_eval),
    EXTERNAL_FUNCTION_ITEM(_ejs_init),
    EXTERNAL_FUNCTION_ITEM(_ejs_object_getprop_utf8),
    EXTERNAL_FUNCTION_ITEM(_ejs_object_setprop_utf8),
    EXTERNAL_FUNCTION_ITEM(_ejs_object_define_value_property),
    EXTERNAL_FUNCTION_ITEM(_ejs_object_define_getter_property),
    EXTERNAL_FUNCTION_ITEM(_ejs_object_define_setter_property),
    EXTERNAL_FUNCTION_ITEM(_ejs_Object_create),
    EXTERNAL_FUNCTION_ITEM(_ejs_Object_getOwnPropertyNames),
    EXTERNAL_FUNCTION_ITEM(_ejs_specop_get),
    EXTERNAL_FUNCTION_ITEM(_ejs_specop_set),
    EXTERNAL_FUNCTION_ITEM(ToEJSBool),
    EXTERNAL_FUNCTION_ITEM(ToDouble),
    EXTERNAL_FUNCTION_ITEM(ToUint32),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_typeof),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_instanceof),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_typeof_is_array),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_plusplus),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_minusminus),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_bitwise_xor),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_bitwise_and),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_bitwise_or),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_rsh),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_ursh),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_lsh),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_ulsh),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_mod),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_add),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_sub),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_mult),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_div),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_lt),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_le),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_gt),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_ge),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_strict_eq),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_strict_neq),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_eq),
    EXTERNAL_FUNCTION_ITEM(_ejs_op_neq),
    EXTERNAL_FUNCTION_ITEM(_ejs_arguments_new),
    EXTERNAL_FUNCTION_ITEM(_ejs_array_new),
    EXTERNAL_FUNCTION_ITEM(_ejs_string_new_utf8_len),
    EXTERNAL_FUNCTION_ITEM(_ejs_function_new_utf8),
    EXTERNAL_FUNCTION_ITEM(_ejs_invoke_closure),
    EXTERNAL_FUNCTION_ITEM(_ejs_regexp_new_utf8),
    {0, 0}
};

#define EXTERNAL_VARIABLE_ITEM(v) {(void *)&v, #v, NULL}
#define EXTERNAL_VARIABLE_ITEM2(v,n) {(void *)&v, #v, n}
ExternalVar externalVariables[] =
{
    EXTERNAL_VARIABLE_ITEM(jsValue32Size),
    EXTERNAL_VARIABLE_ITEM(jsValue32PayloadPart),
    EXTERNAL_VARIABLE_ITEM(jsValue32TagPart),
    EXTERNAL_VARIABLE_ITEM(isLitterEndian),
    EXTERNAL_VARIABLE_ITEM(_ejs_nan),
    EXTERNAL_VARIABLE_ITEM(jsPositiveInfinity),
    EXTERNAL_VARIABLE_ITEM(jsNegativeInfinity),
    EXTERNAL_VARIABLE_ITEM(jsMax),
    EXTERNAL_VARIABLE_ITEM(jsMin),
    EXTERNAL_VARIABLE_ITEM(_ejs_null),
    EXTERNAL_VARIABLE_ITEM(_ejs_undefined),
    EXTERNAL_VARIABLE_ITEM(_ejs_true),
    EXTERNAL_VARIABLE_ITEM(_ejs_false),
    EXTERNAL_VARIABLE_ITEM(_ejs_one),
    EXTERNAL_VARIABLE_ITEM(_ejs_zero),
    EXTERNAL_VARIABLE_ITEM2(_ejs_global, "global"),
    EXTERNAL_VARIABLE_ITEM2(_ejs_console, "console"),
    EXTERNAL_VARIABLE_ITEM2(_ejs_Object, "Object"),
    EXTERNAL_VARIABLE_ITEM2(_ejs_Boolean, "Boolean"),
    EXTERNAL_VARIABLE_ITEM2(_ejs_Number, "Number"),
    EXTERNAL_VARIABLE_ITEM2(_ejs_String, "String"),
    EXTERNAL_VARIABLE_ITEM2(_ejs_Array, "Array"),
    EXTERNAL_VARIABLE_ITEM2(_ejs_Function, "Function"),
    EXTERNAL_VARIABLE_ITEM2(_ejs_Process, "process"),
    EXTERNAL_VARIABLE_ITEM(_ejs_Symbol_create),
    EXTERNAL_VARIABLE_ITEM2(_ejs_Math, "Math"),
    EXTERNAL_VARIABLE_ITEM2(_ejs_JSON, "JSON"),
#if 0
    EXTERNAL_VARIABLE_ITEM(ObjectPrototype),
    EXTERNAL_VARIABLE_ITEM(BooleanObjectPrototype),
    EXTERNAL_VARIABLE_ITEM(NumberObjectPrototype),
    EXTERNAL_VARIABLE_ITEM(StringObjectPrototype),
    EXTERNAL_VARIABLE_ITEM(ArrayObjectPrototype),
    EXTERNAL_VARIABLE_ITEM(FunctionObjectPrototype),
    EXTERNAL_VARIABLE_ITEM(BooleanPrototype),
    EXTERNAL_VARIABLE_ITEM(NumberPrototype),
    EXTERNAL_VARIABLE_ITEM(StringPrototype),
    EXTERNAL_VARIABLE_ITEM(ArrayPrototype),
    EXTERNAL_VARIABLE_ITEM(FunctionPrototype),
#endif
    {0, 0}
};

void initExternals(CodeGenContext &context, ExecutionEngine *ee)
{
    ExternalVar *externalPtr;
    for (externalPtr = externalFunctions; (NULL != externalPtr->external) && (NULL != externalPtr->realname); externalPtr++)
    {
    	llvm::Function *function = context.getFunction(externalPtr->realname);
    	if (NULL == function)
    	{
            std::cout <<  __FUNCTION__ << " Can't find " << externalPtr->realname << "()" << std::endl;
    		context.abort();
    		return;
    	}
    	else
    	{
            ee->addGlobalMapping(function, externalPtr->external);
    	}
    }

    llvm::Module *module = context.getModule();
    for (externalPtr = externalVariables; (NULL != externalPtr->external) && (NULL != externalPtr->realname); externalPtr++)
    {
    	llvm::GlobalVariable *varRef = module->getNamedGlobal(externalPtr->realname);
    	if (NULL == varRef)
    	{
            std::cout <<  __FUNCTION__ << " Can't find variable " << externalPtr->realname << std::endl;
    		context.abort();
    		return;
    	}
    	else
    	{
            ee->addGlobalMapping(varRef, externalPtr->external);
    	}
    }
}

void registVariables(CodeGenContext& context)
{
    //init globalRefs
    ExternalVar *externalPtr;
    llvm::Module *module = context.getModule();
    for (externalPtr = externalVariables; (NULL != externalPtr->external) && (NULL != externalPtr->realname); externalPtr++)
    {
    	llvm::Value *varRef = module->getNamedGlobal(externalPtr->realname);
    	if (NULL == varRef)
    	{
            std::cout <<  __FUNCTION__ << " Can't find variable " << externalPtr->realname << std::endl;
    		context.abort();
    	}
    	else if (NULL == externalPtr->showname)
    	{
    	    context.setVariableRef(externalPtr->realname, varRef, false);
    	}
    	else
    	{
    	    context.setVariableRef(externalPtr->showname, varRef, false);
    	}
    }
    //init globalRefs
    llvm::Value *globalRef = context.getVariableRef("global");
    context.setVariableRef("this", globalRef, false);
    context.setVariableRef("env.addr", globalRef, false);
#if 0
    //init objectTypes
    objectTypes[ObjectRef] = getTypeRef("struct.JSObjectType");
    objectTypes[BooleanRef] = getTypeRef("struct.EJSFunctionType");
    objectTypes[NumberRef] = getTypeRef("struct.JSNumberType");
    objectTypes[StringRef] = getTypeRef("struct.JSStringType");
    objectTypes[ArrayRef] = getTypeRef("struct.JSArrayType");
    objectTypes[FunctionRef] = getTypeRef("struct.EJSFunctionType");
    objectTypes[ObjectPrototypeRef] = getTypeRef("struct.JSPrototype");
    objectTypes[BooleanObjectPrototypeRef] = getTypeRef("struct.JSPrototype");
    objectTypes[NumberObjectPrototypeRef] = getTypeRef("struct.JSPrototype");
    objectTypes[StringObjectPrototypeRef] = getTypeRef("struct.JSPrototype");
    objectTypes[ArrayObjectPrototypeRef] = getTypeRef("struct.JSPrototype");
    objectTypes[FunctionObjectPrototypeRef] = getTypeRef("struct.JSPrototype");
    objectTypes[BooleanPrototypeRef] = getTypeRef("struct.JSBooleanPrototype");
    objectTypes[NumberPrototypeRef] = getTypeRef("struct.JSNumberPrototype");
    objectTypes[StringPrototypeRef] = getTypeRef("struct.JSStringPrototype");
    objectTypes[ArrayPrototypeRef] = getTypeRef("struct.JSArrayPrototype");
    objectTypes[FunctionPrototypeRef] = getTypeRef("struct.EJSFunctionPrototype");
#endif
#if 0
    function = getFunction("JSObjectSetRealType");
	if (NULL == function)
	{
	    std::cout << "Can't find JSObjectSetRealType()" << std::endl;
	    return ;
	}

    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
	llvm::Type *jsType = getTypeRef("struct.JSObject");
    llvm::Type *JSObjectPtrType = llvm::PointerType::get(jsType, 0);
	std::vector<llvm::Value*> args;
	llvm::Value *asObjectRef;

    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, ObjectRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSObjectType");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, BooleanRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.EJSFunctionType");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, NumberRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSNumberType");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, StringRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSStringType");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, ArrayRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSArrayType");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, FunctionRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.EJSFunctionType");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, ObjectPrototypeRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSPrototype");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, BooleanObjectPrototypeRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSPrototype");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, NumberObjectPrototypeRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSPrototype");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, StringObjectPrototypeRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSPrototype");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, ArrayObjectPrototypeRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSPrototype");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, FunctionObjectPrototypeRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSPrototype");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, BooleanPrototypeRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSBooleanPrototype");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, NumberPrototypeRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSNumberPrototype");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, StringPrototypeRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSStringPrototype");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, ArrayPrototypeRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.JSArrayPrototype");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());

    args.clear();
    asObjectRef = llvm::CastInst::Create(llvm::Instruction::BitCast, FunctionPrototypeRef, JSObjectPtrType, "asObjectRef", currentBlock());
    jsType = getTypeRef("struct.EJSFunctionPrototype");
	args.push_back(asObjectRef);
	args.push_back(llvm::ConstantInt::get(int32Type, (int)jsType));
	llvm::CallInst::Create(function, makeArrayRef(args), "", currentBlock());
#endif
}

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
using namespace std;

llvm::Value *CodeGenJSObjectFunctionCall(CodeGenContext& context, llvm::Function *function, std::vector<llvm::Value *>& args)
{
    if (function == NULL)
    {
        std::cout <<  __FUNCTION__ << " BADPARAM function=NULL" << std::endl;
        context.abort();
        return NULL;
    }
    int argn = args.size();
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::ArrayType* arrayType = llvm::ArrayType::get(jsValueType, argn < 1 ? 1 : argn);
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    
    llvm::Value* envRef = context.getVariableRef("env.addr");
    llvm::Value* thisRef = context.getVariableRef("this");
    llvm::Value* argvRef;
    
    if (NULL == envRef)
    {
        envRef = CodeGenJSValue32ConstructForUndefined(context, "env.addr");
    }
    if (NULL == thisRef)
    {
        thisRef = CodeGenJSValue32ConstructForUndefined(context, "this");
    }
    if (argn <= 0)
    {
        argvRef = llvm::Constant::getNullValue(jsValuePtrType);
    }
    else
    {
        AllocaInst* arrayRef = new AllocaInst(arrayType, "array", context.currentBlock());
        arrayRef->setAlignment(8);
        argvRef = new BitCastInst(arrayRef, jsValuePtrType, "", context.currentBlock());
        
        int index = 0;
        std::vector<llvm::Value *>::iterator it;
        for (it = args.begin(); it != args.end(); it++,index++)
        {
            //init parameter
            llvm::Value* indexValue = ConstantInt::get(int32Type, index);
            GetElementPtrInst* argvIndex = GetElementPtrInst::Create(argvRef, indexValue, "argvIndex", context.currentBlock());
    
            //the raw param maybe not ejsval type, so translate if not
            CodeGenJSValue32FunctionCopy(context, argvIndex, *it);
        }
    }

    //begin to call
    std::vector<RegisterID*> realArgs;
    realArgs.push_back(envRef);
    realArgs.push_back(thisRef);
    realArgs.push_back(ConstantInt::get(int32Type, argn));
    realArgs.push_back(argvRef);
    CallInst *call = CallInst::Create(function, makeArrayRef(realArgs), "", context.currentBlock());
    {
        call->setCallingConv(CallingConv::C);
        call->setTailCall(false);

        AttributeSet call_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;

        AttrBuilder B;
        B.addAttribute(Attribute::ByVal);
        B.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B);
        Attrs.push_back(PAS);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 2U, B);
        Attrs.push_back(PAS);

        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        call->setAttributes(call_PAL);
    }

    return call;
}

llvm::Value *CodeGenJSObjectConstructCall(CodeGenContext& context, llvm::Value *objectRef, llvm::Value *thisRef, std::vector<llvm::Value *>& args)
{
    Function *function = context.getFunction("_ejs_invoke_closure");
    if (function == NULL)
    {
        std::cout <<  __FUNCTION__ << " Can't find _ejs_invoke_closure()" << std::endl;
        context.abort();
        return NULL;
    }
    int argn = args.size();
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::ArrayType* arrayType = llvm::ArrayType::get(jsValueType, argn < 1 ? 1 : argn);
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    llvm::Value* argvRef;

    if (argn <= 0)
    {
        argvRef = llvm::Constant::getNullValue(jsValuePtrType);
    }
    else
    {
        AllocaInst* arrayRef = new AllocaInst(arrayType, "array", context.currentBlock());
        argvRef = new BitCastInst(arrayRef, jsValuePtrType, "", context.currentBlock());
        
        int index = 0;
        std::vector<llvm::Value *>::iterator it;
        for (it = args.begin(); it != args.end(); it++,index++)
        {
            //init parameter
            llvm::Value* indexValue = ConstantInt::get(int32Type, index);
            GetElementPtrInst* argvIndex = GetElementPtrInst::Create(argvRef, indexValue, "argvIndex", context.currentBlock());
    
            //the raw param maybe not ejsval type, so translate if not
            CodeGenJSValue32FunctionCopy(context, argvIndex, *it);
        }
    }

    //begin to call
    std::vector<RegisterID*> realArgs;
    realArgs.push_back(objectRef);
    realArgs.push_back(thisRef);
    realArgs.push_back(ConstantInt::get(int32Type, argn));
    realArgs.push_back(argvRef);
    CallInst *call = CallInst::Create(function, makeArrayRef(realArgs), "", context.currentBlock());
    {
        call->setCallingConv(CallingConv::C);
        call->setTailCall(false);

        AttributeSet call_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;

        AttrBuilder B;
        B.addAttribute(Attribute::ByVal);
        B.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B);
        Attrs.push_back(PAS);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 2U, B);
        Attrs.push_back(PAS);

        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        call->setAttributes(call_PAL);
    }

    return call;
}

llvm::Value *CodeGenJSObjectGetPropertyCall(CodeGenContext& context, llvm::Value *objectRef, const std::string& name)
{
    llvm::Function *function = context.getFunction("_ejs_object_getprop_utf8");
    if ((NULL == objectRef) || (NULL == function) || (name.length() <= 0))
    {
        std::cout <<  __FUNCTION__ << " objectRef=" << objectRef << ",function=" << function << ",nameLength=" << name.length() << std::endl;
        return NULL;
    }

    //check objectRef type, it should be (union.ejsval *), but it may be int64_t since ABI can't return struct types
    objectRef = CodeGenJSValue32ConstructIfNeeded(context, objectRef);

    llvm::Type *int8Type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Constant *zero = llvm::Constant::getNullValue(int32Type);
    std::vector<llvm::Constant*> indices;

    indices.push_back(zero);
    indices.push_back(zero);

    llvm::Constant *constArray = llvm::ConstantDataArray::getString(llvm::getGlobalContext(), name);
    llvm::ArrayType* arrayType = llvm::ArrayType::get(int8Type, name.length() + 1);
    llvm::GlobalVariable *globalVariable = new llvm::GlobalVariable(*context.getModule(), arrayType, true, llvm::GlobalValue::PrivateLinkage, constArray, name);
    llvm::Constant *nameRef = llvm::ConstantExpr::getGetElementPtr(globalVariable, indices);

    std::vector<llvm::Value *> args;
    args.push_back(objectRef);
    args.push_back(nameRef);
    llvm::CallInst *call = llvm::CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    {
        call->setCallingConv(CallingConv::C);
        call->setTailCall(false);

        AttributeSet call_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;

        AttrBuilder B;
        B.addAttribute(Attribute::ByVal);
        B.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B);
        Attrs.push_back(PAS);
        
        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        call->setAttributes(call_PAL);
    }

    return call;
}

llvm::Value *CodeGenJSObjectGetPropertyCall(CodeGenContext& context, llvm::Value *objectRef, llvm::Value *propertyRef)
{
    llvm::Function *function = context.getFunction("_ejs_specop_get");
    if ((NULL == objectRef) || (NULL == function))
    {
        std::cout <<  __FUNCTION__ << " objectRef=" << objectRef << ",function=" << function << std::endl;
        return NULL;
    }
    
    //check objectRef type, it should be (union.ejsval *), but it may be int64_t since ABI can't return struct types
    objectRef = CodeGenJSValue32ConstructIfNeeded(context, objectRef);
    propertyRef = CodeGenJSValue32ConstructIfNeeded(context, propertyRef);

    std::vector<RegisterID*> args;
    args.push_back(objectRef);
    args.push_back(propertyRef);
    args.push_back(objectRef);
    llvm::CallInst *call = llvm::CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    {
        call->setCallingConv(CallingConv::C);
        call->setTailCall(false);

        AttributeSet call_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;
        
        AttrBuilder B;
        B.addAttribute(Attribute::ByVal);
        B.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B);
        Attrs.push_back(PAS);
        
        PAS = AttributeSet::get(llvm::getGlobalContext(), 2U, B);
        Attrs.push_back(PAS);
        
        PAS = AttributeSet::get(llvm::getGlobalContext(), 3U, B);
        Attrs.push_back(PAS);
        
        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        call->setAttributes(call_PAL);
    }

    return call;
}

llvm::Value *CodeGenJSObjectSetPropertyCall(CodeGenContext& context, llvm::Value *objectRef, const std::string& name, llvm::Value *valueRef)
{
    llvm::Function *function = context.getFunction("_ejs_object_setprop_utf8");
    if ((NULL == objectRef) || (NULL == valueRef) || (NULL == function) || (name.length() <= 0))
    {
        std::cout <<  __FUNCTION__ << " objectRef=" << objectRef << ",valueRef=" << valueRef << ",function=" << function << ",nameLength=" << name.length() << std::endl;
        return NULL;
    }
    
    //check objectRef type, it should be (union.ejsval *), but it may be int64_t since ABI can't return struct types
    objectRef = CodeGenJSValue32ConstructIfNeeded(context, objectRef);
    valueRef = CodeGenJSValue32ConstructIfNeeded(context, valueRef);

    llvm::Type *int8Type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Constant *zero = llvm::Constant::getNullValue(int32Type);
    std::vector<llvm::Constant*> indices;

    indices.push_back(zero);
    indices.push_back(zero);

    llvm::Constant *constArray = llvm::ConstantDataArray::getString(llvm::getGlobalContext(), name);
    llvm::ArrayType* arrayType = llvm::ArrayType::get(int8Type, name.length() + 1);
    llvm::GlobalVariable *globalVariable = new llvm::GlobalVariable(*context.getModule(), arrayType, true, llvm::GlobalValue::PrivateLinkage, constArray, name);
    llvm::Constant *nameRef = llvm::ConstantExpr::getGetElementPtr(globalVariable, indices);

    std::vector<llvm::Value *> args;
    args.push_back(objectRef);
    args.push_back(nameRef);
    args.push_back(valueRef);
    llvm::CallInst *call = llvm::CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    {
        call->setCallingConv(CallingConv::C);
        call->setTailCall(false);

        AttributeSet call_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;
        
        AttrBuilder B;
        B.addAttribute(Attribute::ByVal);
        B.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B);
        Attrs.push_back(PAS);
        
        PAS = AttributeSet::get(llvm::getGlobalContext(), 3U, B);
        Attrs.push_back(PAS);
        
        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        call->setAttributes(call_PAL);
    }
    
    return call;
}

llvm::Value *CodeGenJSObjectSetPropertyCall(CodeGenContext& context, llvm::Value *objectRef, llvm::Value *propertyRef, llvm::Value *valueRef)
{
    llvm::Function *function = context.getFunction("_ejs_specop_set");
    if ((NULL == objectRef) || (NULL == valueRef) || (NULL == function))
    {
        std::cout <<  __FUNCTION__ << " objectRef=" << objectRef << ",valueRef=" << valueRef << ",function=" << function << std::endl;
        return NULL;
    }
    
    //check objectRef type, it should be (union.ejsval *), but it may be int64_t since ABI can't return struct types
    objectRef = CodeGenJSValue32ConstructIfNeeded(context, objectRef);
    propertyRef = CodeGenJSValue32ConstructIfNeeded(context, propertyRef);
    valueRef = CodeGenJSValue32ConstructIfNeeded(context, valueRef);

    std::vector<RegisterID*> args;
    args.push_back(objectRef);
    args.push_back(propertyRef);
    args.push_back(valueRef);
    args.push_back(objectRef);
    llvm::CallInst *call = llvm::CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    {
        call->setCallingConv(CallingConv::C);
        call->setTailCall(false);

        AttributeSet call_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;
        
        AttrBuilder B0;
        B0.addAttribute(Attribute::ZExt);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 0U, B0);
        Attrs.push_back(PAS);

        AttrBuilder B1;
        B1.addAttribute(Attribute::ByVal);
        B1.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B1);
        Attrs.push_back(PAS);
        
        PAS = AttributeSet::get(llvm::getGlobalContext(), 2U, B1);
        Attrs.push_back(PAS);
        
        PAS = AttributeSet::get(llvm::getGlobalContext(), 3U, B1);
        Attrs.push_back(PAS);
        
        PAS = AttributeSet::get(llvm::getGlobalContext(), 4U, B1);
        Attrs.push_back(PAS);
        
        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        call->setAttributes(call_PAL);
    }
    
    return call;
}

llvm::Value *CodeGenJSObjectTypeOfCall(CodeGenContext& context, llvm::Value *objectRef)
{
    llvm::Function *function = context.getFunction("_ejs_op_typeof");
    if ((NULL == objectRef) || (NULL == function))
    {
        std::cout <<  __FUNCTION__ << " objectRef=" << objectRef << ",function=" << function << std::endl;
        return NULL;
    }
    
    //check objectRef type, it should be (union.ejsval *), but it may be int64_t since ABI can't return struct types
    objectRef = CodeGenJSValue32ConstructIfNeeded(context, objectRef);

    std::vector<RegisterID*> args;
    args.push_back(objectRef);
    llvm::CallInst *call = llvm::CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    {
        call->setCallingConv(CallingConv::C);
        call->setTailCall(false);

        AttributeSet call_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;
        
        AttrBuilder B0;
        B0.addAttribute(Attribute::ZExt);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 0U, B0);
        Attrs.push_back(PAS);

        AttrBuilder B1;
        B1.addAttribute(Attribute::ByVal);
        B1.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B1);
        Attrs.push_back(PAS);
        
        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        call->setAttributes(call_PAL);
    }
    
    return call;
}

llvm::Value *CodeGenJSObjectInstanceOfCall(CodeGenContext& context, llvm::Value *objectRef, llvm::Value *typeRef)
{
    llvm::Function *function = context.getFunction("_ejs_op_instanceof");
    if ((NULL == objectRef) || (NULL == typeRef) || (NULL == function))
    {
        std::cout <<  __FUNCTION__ << " objectRef=" << objectRef << ",typeRef=" << typeRef << ",function=" << function << std::endl;
        return NULL;
    }
    
    //check objectRef type, it should be (union.ejsval *), but it may be int64_t since ABI can't return struct types
    objectRef = CodeGenJSValue32ConstructIfNeeded(context, objectRef);
    typeRef = CodeGenJSValue32ConstructIfNeeded(context, typeRef);

    std::vector<RegisterID*> args;
    args.push_back(objectRef);
    args.push_back(typeRef);
    llvm::CallInst *call = llvm::CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    {
        call->setCallingConv(CallingConv::C);
        call->setTailCall(false);

        AttributeSet call_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;
        
        AttrBuilder B;
        B.addAttribute(Attribute::ByVal);
        B.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B);
        Attrs.push_back(PAS);
        
        PAS = AttributeSet::get(llvm::getGlobalContext(), 2U, B);
        Attrs.push_back(PAS);
        
        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        call->setAttributes(call_PAL);
    }

    return call;
}

//APIs for Create Object, String, Function, Array
llvm::Value *CodeGenJSObjectMakeArrayCall(CodeGenContext& context, llvm::Value *objectRef, llvm::Value *typeRef)
{
    Function *function = context.getFunction("_ejs_array_new");
    if (NULL == function)
    {
        std::cout <<  __FUNCTION__ << " Can't find _ejs_array_new()" << std::endl;
        context.abort();
        return NULL;
    }

    return NULL;
}

llvm::Value *CodeGenJSObjectMakeStringCall(CodeGenContext& context, const char *string, int length)
{
    //FIXME: should we search it before create a new GlobalVariable ?
    //that is why I make it a function here
    llvm::Constant *varRef;

    llvm::IntegerType *int8Type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
    llvm::Type *int8PtrType = llvm::PointerType::get(int8Type, 0);
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    if ((NULL == string) || (length <= 0))
    {
        //ensure params
        length = 0;
        varRef = llvm::Constant::getNullValue(int8PtrType);
    }
    else
    {
        llvm::Constant *zero = llvm::Constant::getNullValue(int32Type);
        std::vector<llvm::Constant*> indices;
    
        indices.push_back(zero);
        indices.push_back(zero);
    
        llvm::Constant *constArray = llvm::ConstantDataArray::getString(llvm::getGlobalContext(), string);
        llvm::ArrayType* arrayType = llvm::ArrayType::get(int8Type, length + 1);
        llvm::GlobalVariable *globalVariable = new llvm::GlobalVariable(*context.getModule(), arrayType, true, llvm::GlobalValue::PrivateLinkage, constArray, string);
        varRef = llvm::ConstantExpr::getGetElementPtr(globalVariable, indices);
    }

    llvm::Function *function = context.getFunction("_ejs_string_new_utf8_len");
    if ((NULL == varRef) || (NULL == function))
    {
        std::cout <<  __FUNCTION__ << " varRef=" << varRef << ",function=" << function << std::endl;
        return NULL;
    }
    std::vector<RegisterID*> args;
    args.push_back(varRef);
    args.push_back(ConstantInt::get(int32Type, length, true));
    llvm::CallInst *call = llvm::CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    {
        call->setCallingConv(CallingConv::C);
        call->setTailCall(false);

        AttributeSet call_PAL;
        call->setAttributes(call_PAL);
    }
    
    return call;
}

llvm::Value *CodeGenJSObjectMakeFunctionCall(CodeGenContext& context, llvm::Function *function, const std::string& name)
{
    //check whether create or not
    llvm::Value *oldRef = context.getVariableRef(name, false);
    if ((oldRef != function) && (NULL != oldRef))
    {
        //Need I check the type of oldRef ? syntax should do this, but I need move reserved variables to an individual map
        return oldRef;
    }

    //find this
    llvm::Value *thisRef = context.getVariableRef("this");
    //set this.m_ident
    llvm::Function *_ejs_function = context.getFunction("_ejs_function_new_utf8");
    if ((NULL == thisRef) || (NULL == _ejs_function))
    {
        std::cout <<  __FUNCTION__ << " thisRef=" << thisRef << ",function=" << _ejs_function << std::endl;
        return NULL;
    }

    std::vector<RegisterID*> args;
    llvm::Type *int8Type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
    llvm::Type *int8PtrType = llvm::PointerType::get(int8Type, 0);
    llvm::Constant *nameRef;
    if (name.length() <= 0)
    {
        nameRef = llvm::Constant::getNullValue(int8PtrType);
    }
    else
    {
        llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
        llvm::Constant *zero = llvm::Constant::getNullValue(int32Type);
        std::vector<llvm::Constant*> indices;
    
        indices.push_back(zero);
        indices.push_back(zero);
    
        llvm::Constant *constArray = llvm::ConstantDataArray::getString(llvm::getGlobalContext(), name);
        llvm::ArrayType* arrayType = llvm::ArrayType::get(int8Type, name.length() + 1);
        llvm::GlobalVariable *globalVariable = new llvm::GlobalVariable(*context.getModule(), arrayType, true, llvm::GlobalValue::PrivateLinkage, constArray, name);
        nameRef = llvm::ConstantExpr::getGetElementPtr(globalVariable, indices);
    }

    args.push_back(thisRef);
    args.push_back(nameRef);
    args.push_back(function);
    llvm::CallInst *call = llvm::CallInst::Create(_ejs_function, makeArrayRef(args), "", context.currentBlock());
    {
        call->setCallingConv(CallingConv::C);
        call->setTailCall(false);

        AttributeSet call_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;
        
        AttrBuilder B;
        B.addAttribute(Attribute::ByVal);
        B.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B);
        Attrs.push_back(PAS);
        
        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        call->setAttributes(call_PAL);
    }

    //for anonymous function
    if (name.length() <= 0)
    {
        return call;
    }
    
    //Create a GlobalVariable for the function
    //FIXME: the initialization should be done in a early function
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Value *functionRef = new GlobalVariable(*context.getModule(), jsValueType, false, GlobalValue::ExternalLinkage, ConstantAggregateZero::get(jsValueType), name);
    CodeGenJSValue32FunctionCopy(context, functionRef, call);
    context.setVariableRef(name, functionRef, false);

    return functionRef;
}


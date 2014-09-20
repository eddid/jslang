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

#ifndef CodeGen_h
#define CodeGen_h

#include <stack>
#include <typeinfo>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/PrintModulePass.h>
//#include <llvm/ModuleProvider.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/raw_ostream.h>

#include "ejs-value.h"
#include "Nodes.h"
#include "Identifiers.h"
#include <math.h>
#if defined(WIN32)
inline bool isinf(double num) { return !_finite(num) && !_isnan(num); }
inline bool isnan(double num) { return !!_isnan(num); }
inline double fmod1(double x, double y)
{
    int i = (int)floor(x/y);
    return x - i * y;
}
#endif

using namespace llvm;
using namespace JSC;

class CodeGenBlock {
public:
    CodeGenBlock(CodeGenBlock *last = NULL, llvm::BasicBlock *bBlock = NULL, const std::string &bName = std::string())
    : parent(last)
    , block(bBlock)
    , breakBlock(NULL)
    , continueBlock(NULL)
    , name(bName)
    {
    }
    
public:
    CodeGenBlock *parent;
    llvm::BasicBlock *block;
    llvm::BasicBlock *breakBlock;
    llvm::BasicBlock *continueBlock;
    const std::string name;
    std::map<std::string, llvm::Value*> localRefs;
    std::map<std::string, llvm::Value*> localConstRefs;
};

class CodeGenContext
{
public:
    CodeGenContext(llvm::Module *m = NULL)
        : module(m)
        , topBlock(NULL)
        , mainFunc(NULL)
     {
         if (NULL == module)
         {
             module = new Module("main", getGlobalContext());
         }
         propertyNames = new Identifers();
     }
    
    void init();
    void generateCode(SourceElements& root);
    GenericValue runCode();
    void compileCode(char *inputFile);
    void abort();
    
    bool isGlobalVariable(const std::string &id);
    bool isConstVariable(const std::string &id);
    /**
     * clone variables form predecessor for cloure
     */
    void cloneVariable();
    llvm::Value *getVariable(const std::string &id);
    llvm::Value *getVariableRef(const std::string &id, bool inLocal = false);
    void setVariableRef(const std::string &id, llvm::Value *varRef, bool inLocal = true, bool inConst = false);
    llvm::Type *getTypeRef(const std::string &name);
    void setTypeRef(const std::string &name, llvm::Type *typeRef, bool inLocal = true);
    llvm::Type *getObjectType(llvm::Value *object);
    void setObjectType(llvm::Value *object, llvm::Type *type);
    /**
     * break or else block
     * it is break blockwhen deal with break syntax in while or for
     * it is else block when deal with if (logical check)
     */
    void setBreakBlock(llvm::BasicBlock *block);
    llvm::BasicBlock *getBreakBlock();
    /**
     * continue or if block
     * it is continue block when deal with break syntax in while or for
     * it is if block when deal with if (logical check)
     */
    void setContinueBlock(llvm::BasicBlock *block);
    llvm::BasicBlock *getContinueBlock();
    void pushBlock(llvm::BasicBlock *block, const std::string &name = std::string());
    /**
     * @param block: pop block until meet block
     */
    void popBlock(llvm::BasicBlock *block = NULL);

    //FIXME: these should be removed
    llvm::BasicBlock *findBlock(const std::string &name);
    
    llvm::BasicBlock *currentBlock() { return topBlock->block; }
    llvm::Function *currentFunction() { return topBlock->block->getParent(); }
    llvm::Function *mainFunction() { return mainFunc; }
    llvm::Function *getFunction(const char *name) { return module->getFunction(name); }
    llvm::Module *getModule() { return module; }
    llvm::Value* emitPutById(llvm::Value* obj, std::string &name, llvm::Value* value) { return NULL; }
    llvm::Value* emitPutGetter(llvm::Value* obj, std::string &name, llvm::Value* value) { return NULL; }
    llvm::Value* emitPutSetter(llvm::Value* obj, std::string &name, llvm::Value* value) { return NULL; }

public:
    Identifers *propertyNames;

private:
    llvm::Module *module;
    llvm::Function *mainFunc;
    CodeGenBlock *topBlock;
    
    std::vector<CodeGenBlock *> blocks;
    std::map<std::string, llvm::Value*> globalRefs;
    std::map<std::string, llvm::Value*> globalConstRefs;
    std::map<std::string, llvm::Type*> globalTypes;
    std::map<llvm::Value*, llvm::Type*> objectTypes;
};

static const double D32 = 4294967296.0;
static inline double trunc(double num) { return num > 0 ? floor(num) : ceil(num); }

static inline int32_t toInt32SlowCase(double d, bool& ok)
{
    ok = true;

    if (d >= -D32 / 2 && d < D32 / 2)
        return static_cast<int32_t>(d);

    if (isnan(d) || isinf(d)) {
        ok = false;
        return 0;
    }

    double d32 = fmod(trunc(d), D32);
    if (d32 >= D32 / 2)
        d32 -= D32;
    else if (d32 < -D32 / 2)
        d32 += D32;
    return static_cast<int32_t>(d32);
}

static inline uint32_t toUInt32SlowCase(double d, bool& ok)
{
    ok = true;

    if (d >= 0.0 && d < D32)
        return static_cast<uint32_t>(d);

    if (isnan(d) || isinf(d)) {
        ok = false;
        return 0;
    }

    double d32 = fmod(trunc(d), D32);
    if (d32 < 0)
        d32 += D32;
    return static_cast<uint32_t>(d32);
}

inline int32_t toInt32(double val)
{
    if (!(val >= -2147483648.0 && val < 2147483648.0)) {
        bool ignored;
        return toInt32SlowCase(val, ignored);
    }
    return static_cast<int32_t>(val);
}

inline uint32_t toUInt32(double val)
{
    if (!(val >= 0.0 && val < 4294967296.0)) {
        bool ignored;
        return toUInt32SlowCase(val, ignored);
    }
    return static_cast<uint32_t>(val);
}

llvm::Type *CodeGenJSValueTypes(CodeGenContext& context);
llvm::Value *CodeGenJSValue32ConstructIfNeeded(CodeGenContext& context, llvm::Value *jsValue);
llvm::Value *CodeGenJSValue32Construct(CodeGenContext& context, const char *name);
llvm::Value *CodeGenJSValue32Destruct(CodeGenContext& context, llvm::Value *jsValue);
llvm::Value *CodeGenJSValue32ConstructForNULL(CodeGenContext& context, const char *name = "");
llvm::Value *CodeGenJSValue32ConstructForUndefined(CodeGenContext& context, const char *name = "");
llvm::Value *CodeGenJSValue32ConstructForBool(CodeGenContext& context, bool value, const char *name = "");
llvm::Value *CodeGenJSValue32ConstructForBool(CodeGenContext& context, llvm::Value *value, const char *name = "");
llvm::Value *CodeGenJSValue32ConstructForInt32(CodeGenContext& context, int value, const char *name = "");
llvm::Value *CodeGenJSValue32ConstructForDouble(CodeGenContext& context, double value, const char *name = "");
llvm::Value *CodeGenJSValue32ConstructForObject(CodeGenContext& context, llvm::Value *jsObjRef, const char *name = "");
llvm::Value *CodeGenJSValue32FunctionIsUndefined(CodeGenContext& context, llvm::Value *jsValue);
llvm::Value *CodeGenJSValue32FunctionIsTrue(CodeGenContext& context, llvm::Value *jsValue);
llvm::Value *CodeGenJSValue32FunctionToBool(CodeGenContext& context, llvm::Value *jsValue);
llvm::Value *CodeGenJSValue32FunctionToDouble(CodeGenContext& context, llvm::Value *jsValue);
llvm::Value *CodeGenJSValue32FunctionToInt32(CodeGenContext& context, llvm::Value *jsValue);
llvm::Value *CodeGenJSValue32FunctionToInt64(CodeGenContext& context, llvm::Value *jsValue);
llvm::Value *CodeGenJSValue32FunctionToString(CodeGenContext& context, llvm::Value *jsValue, llvm::Value *buffer, llvm::Value *bufferLength);
llvm::Value *CodeGenJSValue32FunctionCopy(CodeGenContext& context, llvm::Value *jsValueDesRef, llvm::Value *jsValueSrc);
llvm::Value *CodeGenJSValue32FunctionCmp(CodeGenContext& context, llvm::Value *jsValueLeft, llvm::Value *jsValueRight, OpcodeID option);
llvm::Value *CodeGenJSValue32FunctionOption(CodeGenContext& context, llvm::Value *jsValueLeft, llvm::Value *jsValueRight, OpcodeID option);
llvm::Value *CodeGenJSValue32FunctionGetHashCode(CodeGenContext& context, llvm::Value *jsValueRef);

llvm::Value *CodeGenJSObjectConstructCall(CodeGenContext& context, llvm::Value *objectRef, llvm::Value *thisRef, std::vector<llvm::Value *>& args);
llvm::Value *CodeGenJSObjectFunctionCall(CodeGenContext& context, llvm::Function *function, std::vector<llvm::Value *>& args);
llvm::Value *CodeGenJSObjectGetPropertyCall(CodeGenContext& context, llvm::Value *objectRef, const std::string& name);
llvm::Value *CodeGenJSObjectGetPropertyCall(CodeGenContext& context, llvm::Value *objectRef, llvm::Value *propertyRef);
llvm::Value *CodeGenJSObjectSetPropertyCall(CodeGenContext& context, llvm::Value *objectRef, const std::string& name, llvm::Value *valueRef);
llvm::Value *CodeGenJSObjectSetPropertyCall(CodeGenContext& context, llvm::Value *objectRef, llvm::Value *propertyRef, llvm::Value *valueRef);
llvm::Value *CodeGenJSObjectTypeOfCall(CodeGenContext& context, llvm::Value *objectRef);
llvm::Value *CodeGenJSObjectInstanceOfCall(CodeGenContext& context, llvm::Value *objectRef, llvm::Value *typeRef);
llvm::Value *CodeGenJSObjectMakeStringCall(CodeGenContext& context, const char *string, int length);
llvm::Value *CodeGenJSObjectMakeFunctionCall(CodeGenContext& context, llvm::Function *function, const std::string& name);

#endif // CodeGen_h

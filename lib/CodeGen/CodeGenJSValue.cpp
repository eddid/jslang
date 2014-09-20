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

/**
 * union ejsval {
 * 	int64 asInt64;
 * 	double asDouble;
 * 	int32 asIntArray[2];
 *  struct {
 *      int32 payload;
 *      int32 tag;
 *  } asBits;
 * };
 * asIntArray[jsValue32PayloadPart] stand for payload
 * asIntArray[jsValue32TagPart] stand for value
 *
 * On 32-bit platforms USE(JSVALUE32_64) should be defined, and we use a NaN-encoded
 * form for immediates.
 *
 * The encoding makes use of unused NaN space in the IEEE754 representation.  Any value
 * with the top 13 bits set represents a QNaN (with the sign bit set).  QNaN values
 * can encode a 51-bit payload.  Hardware produced and C-library payloads typically
 * have a payload of zero.  We assume that non-zero payloads are available to encode
 * pointer and integer values.  Since any 64-bit bit pattern where the top 15 bits are
 * all set represents a NaN with a non-zero payload, we can use this space in the NaN
 * ranges to encode other values (however there are also other ranges of NaN space that
 * could have been selected).
 *
 * For JSValues that do not contain a double value, the high 32 bits contain the tag
 * values listed in the enums below, which all correspond to NaN-space. In the case of
 * cell, integer and bool values the lower 32 bits (the 'payload') contain the pointer
 * integer or boolean value; in the case of all other tags the payload is 0.
 */
/**
 * On 64-bit platforms USE(JSVALUE64) should be defined, and we use a NaN-encoded
 * form for immediates.
 *
 * The encoding makes use of unused NaN space in the IEEE754 representation.  Any value
 * with the top 13 bits set represents a QNaN (with the sign bit set).  QNaN values
 * can encode a 51-bit payload.  Hardware produced and C-library payloads typically
 * have a payload of zero.  We assume that non-zero payloads are available to encode
 * pointer and integer values.  Since any 64-bit bit pattern where the top 15 bits are
 * all set represents a NaN with a non-zero payload, we can use this space in the NaN
 * ranges to encode other values (however there are also other ranges of NaN space that
 * could have been selected).
 *
 * This range of NaN space is represented by 64-bit numbers begining with the 16-bit
 * hex patterns 0xFFFE and 0xFFFF - we rely on the fact that no valid double-precision
 * numbers will begin fall in these ranges.
 *
 * The top 16-bits denote the type of the encoded ejsval:
 *
 *     Pointer {  0000:PPPP:PPPP:PPPP
 *              / 0001:****:****:****
 *     Double  {         ...
 *              \ FFFE:****:****:****
 *     Integer {  FFFF:0000:IIII:IIII
 *
 * The scheme we have implemented encodes double precision values by performing a
 * 64-bit integer addition of the value 2^48 to the number. After this manipulation
 * no encoded double-precision value will begin with the pattern 0x0000 or 0xFFFF.
 * Values must be decoded by reversing this operation before subsequent floating point
 * operations my be peformed.
 *
 * 32-bit signed integers are marked with the 16-bit tag 0xFFFF.
 *
 * The tag 0x0000 denotes a pointer, or another form of tagged immediate. Boolean,
 * null and undefined values are represented by specific, invalid pointer values:
 *
 *     False:     0x06
 *     True:      0x07
 *     Undefined: 0x0a
 *     Null:      0x02
 *
 * These values have the following properties:
 * - Bit 1 (TagBitTypeOther) is set for all four values, allowing real pointers to be
 *   quickly distinguished from all immediate values, including these invalid pointers.
 * - With bit 3 is masked out (TagBitUndefined) Undefined and Null share the
 *   same value, allowing null & undefined to be quickly detected.
 *
 * No valid ejsval will have the bit pattern 0x0, this is used to represent array
 * holes, and as a C++ 'no value' result (e.g. ejsval() has an internal value of 0).
 *
 */
// These values are #defines since using static const integers here is a ~1% regression!

// These special values are never visible to JavaScript code; Empty is used to represent
// Array holes, and for uninitialized JSValues. Deleted is used in hash table code.
// These values would map to cell types in the ejsval encoding, but not valid GC cell
// pointer should have either of these values (Empty is null, deleted is at an invalid
// alignment for a GC cell, and in the zero page).

/*************************************************************
* PRIVATE FUNCTIONS                                                  *
**************************************************************/
static void CodeGenJSValue32FunctionCopyForInt64(CodeGenContext& context, llvm::Value *jsValueDesRef, llvm::Value *jsValueSrcInt64)
{
    llvm::Type *int64PtrType = llvm::Type::getInt64PtrTy(llvm::getGlobalContext());
    //int64Ref = (long long*)jsValueRef;
	llvm::Value *int64Ref = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValueDesRef, int64PtrType, "asInt64", context.currentBlock());
    //*int64Ref =value
    new llvm::StoreInst(jsValueSrcInt64, int64Ref, false, 8, context.currentBlock());
}

static void CodeGenJSValue32FunctionCopyForDouble(CodeGenContext& context, llvm::Value *jsValueDesRef, llvm::Value *jsValueSrcDouble)
{
    llvm::Type *doublePtrType = llvm::Type::getDoublePtrTy(llvm::getGlobalContext());
    //doubleRef = (double*)jsValueRef;
	llvm::Value *doubleRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValueDesRef, doublePtrType, "asDouble", context.currentBlock());
    //*doubleRef =value
    new llvm::StoreInst(jsValueSrcDouble, doubleRef, false, 8, context.currentBlock());
}

static void CodeGenJSValue32FunctionCopyForJSValue(CodeGenContext& context, llvm::Value *jsValueDesRef, llvm::Value *jsValueSrcRef)
{
	Function *function = context.getFunction("memcpy");
	if (function == NULL)
	{
		std::cout <<  __FUNCTION__ << " Can't find memcpy()" << std::endl;
		context.abort();
		return ;
	}
    llvm::Type *int8PtrType = llvm::Type::getInt8PtrTy(llvm::getGlobalContext());
    //srcRef = (void*)jsValueSrcRef;
	llvm::Value *srcRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValueSrcRef, int8PtrType, "asSrcPtr", context.currentBlock());
	llvm::Value *desRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValueDesRef, int8PtrType, "asDesPtr", context.currentBlock());
    //*doubleRef =value
	std::vector<RegisterID*> args;
	args.push_back(desRef);
	args.push_back(srcRef);
	//WARNING: should be size in compiling env, rather than compiler env
	args.push_back(context.getVariable("jsValue32Size"));
	llvm::CallInst *call = llvm::CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    call->setCallingConv(CallingConv::C);
    call->setTailCall(false);

}

static void CodeGenJSValue32FunctionCopyForOthers(CodeGenContext& context, llvm::Value *jsValueDesRef, llvm::Value *jsValueSrcTag, llvm::Value *jsValueSrcPayload)
{
    llvm::Type *int32PtrType = llvm::Type::getInt32PtrTy(llvm::getGlobalContext());
    //intRef = (int *)jsValueRef;
	llvm::Value *intRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValueDesRef, int32PtrType, "asIntPtr", context.currentBlock());
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());

	//tagRef = intRef[jsValue32TagPart]
    llvm::Value *tagRef = llvm::GetElementPtrInst::Create(intRef, context.getVariable("jsValue32TagPart"), "tagRef", context.currentBlock());
    //*tagRef = EJSVAL_TAG_INT32
    new llvm::StoreInst(jsValueSrcTag, tagRef, false, 4, context.currentBlock());

	//payloadRef = intRef[jsValue32PayloadPart]
    llvm::Value *payloadRef = llvm::GetElementPtrInst::Create(intRef, context.getVariable("jsValue32PayloadPart"), "payloadRef", context.currentBlock());
    //payloadRef = 0
    new llvm::StoreInst(jsValueSrcPayload, payloadRef, false, 4, context.currentBlock());
}

/*
*/
llvm::Value *CodeGenJSValue32FunctionCopy(CodeGenContext& context, llvm::Value *jsValueDesRef, llvm::Value *jsValueSrc)
{
    if ((NULL == jsValueDesRef) || (NULL == jsValueSrc))
    {
        std::cout <<  __FUNCTION__ << " BADPARAM" << std::endl;
        return NULL;
    }

    if (jsValueDesRef == jsValueSrc)
    {
        return jsValueSrc;
    }

    llvm::Type *srcType = jsValueSrc->getType();
    llvm::Type *desType = jsValueDesRef->getType();
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    if (desType != jsValuePtrType)
    {
        std::cout <<  __FUNCTION__ << " BADTYPE OF jsValueDesRef" << std::endl;
        return NULL;
    }

    llvm::Value *result = NULL;
    int bitWidth;
    switch (srcType->getTypeID())
    {
    case llvm::Type::DoubleTyID:
        CodeGenJSValue32FunctionCopyForDouble(context, jsValueDesRef, jsValueSrc);
        result = jsValueDesRef;
        break;

    case llvm::Type::IntegerTyID:
        bitWidth = cast<llvm::IntegerType>(srcType)->getBitWidth();
        if (64 == bitWidth)
        {
            CodeGenJSValue32FunctionCopyForInt64(context, jsValueDesRef, jsValueSrc);
        }
        else if (32 == bitWidth)
        {
            CodeGenJSValue32FunctionCopyForOthers(context, jsValueDesRef, llvm::ConstantInt::get(srcType, EJSVAL_TAG_INT32), jsValueSrc);
        }
        else if (1 == bitWidth)
        {
            llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
            CodeGenJSValue32Destruct(context, jsValueDesRef);
            RegisterID *int32Result = new ZExtInst(jsValueSrc, int32Type, "", context.currentBlock());
            CodeGenJSValue32FunctionCopyForOthers(context, jsValueDesRef, llvm::ConstantInt::get(int32Type, EJSVAL_TAG_BOOLEAN), int32Result);
        }
        else
        {
            std::cout <<  __FUNCTION__ << " BAD IntegerTyID OF jsValueSrc" << std::endl;
        }
        result = jsValueDesRef;
        break;

    case llvm::Type::StructTyID:
        if (jsValueType == srcType)
        {
            new llvm::StoreInst(jsValueSrc, jsValueDesRef, false, 8, context.currentBlock());
            result = jsValueDesRef;
        }
        else
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE OF jsValueSrc" << std::endl;
        }
        break;

    case llvm::Type::PointerTyID:
        if (jsValuePtrType == srcType)
        {
            CodeGenJSValue32FunctionCopyForJSValue(context, jsValueDesRef, jsValueSrc);
        }
        else if (llvm::isa<llvm::Function>(jsValueSrc))
        {
            llvm::Function *function = (llvm::Function *)jsValueSrc;
            llvm::Value *jsFunctionRef = CodeGenJSObjectMakeFunctionCall(context, function, function->getName());
            CodeGenJSValue32FunctionCopy(context, jsValueDesRef, jsFunctionRef);
        }
        else
        {
	        llvm::Value *jsValueSrcReal = new LoadInst(jsValueSrc, "", false, context.currentBlock());
            result = CodeGenJSValue32FunctionCopy(context, jsValueDesRef, jsValueSrcReal);
        }
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE OF jsValueSrc" << std::endl;
        break;
    }

    if (NULL != result)
    {
        return jsValueSrc;//new llvm::LoadInst(result, "", false, context.currentBlock());
    }

    return NULL;
}

/*
 deal with + - * /, so just cast to double type if needed
*/
static llvm::Value *CodeGenJSValue32FunctionOptionForDouble(CodeGenContext& context, llvm::Value *jsValueLeft, llvm::Value *jsValueRight, OpcodeID option)
{
    if ((NULL == jsValueLeft) || (NULL == jsValueRight))
    {
        std::cout <<  __FUNCTION__ << " BADPARAM" << std::endl;
        return NULL;
    }

    llvm::Type *doubleType = llvm::Type::getDoubleTy(llvm::getGlobalContext());
	Instruction::BinaryOps instr;
    switch (option)
    {
    case op_plusplus:
        instr = Instruction::FAdd;
        jsValueRight = llvm::ConstantFP::get(doubleType, 1.0);
        break;

    case op_minusminus:
        instr = Instruction::FSub;
        jsValueRight = llvm::ConstantFP::get(doubleType, 1.0);
        break;

    case op_add:
        instr = Instruction::FAdd;
        break;

    case op_sub:
        instr = Instruction::FSub;
        break;

    case op_mul:
        instr = Instruction::FMul;
        break;

    case op_div:
        instr = Instruction::FDiv;
        break;

    case op_mod:
        instr = Instruction::FRem;
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE option=" << option << std::endl;
        return NULL;
    }

    llvm::Value *doubleLeft = CodeGenJSValue32FunctionToDouble(context, jsValueLeft);
    llvm::Value *doubleRight = CodeGenJSValue32FunctionToDouble(context, jsValueRight);

    if ((NULL != doubleLeft) && (NULL != doubleRight))
    {
    	return BinaryOperator::Create(instr, doubleLeft, doubleRight, "", context.currentBlock());
    }

    return NULL;
}


/*
 deal with 
*/
static llvm::Value *CodeGenJSValue32FunctionOptionForInt32(CodeGenContext& context, llvm::Value *jsValueLeft, llvm::Value *jsValueRight, OpcodeID option)
{
    if ((NULL == jsValueLeft) || (NULL == jsValueRight))
    {
        std::cout <<  __FUNCTION__ << " BADPARAM" << std::endl;
        return NULL;
    }

    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
	Instruction::BinaryOps instr;
    switch (option)
    {
    case op_plusplus:
        instr = Instruction::Add;
        jsValueRight = llvm::ConstantInt::get(int32Type, 1);
        break;

    case op_minusminus:
        instr = Instruction::Sub;
        jsValueRight = llvm::ConstantInt::get(int32Type, 1);
        break;

    case op_add:
        instr = Instruction::Add;
        break;

    case op_sub:
        instr = Instruction::Sub;
        break;

    case op_mul:
        instr = Instruction::Mul;
        break;

    case op_div:
        instr = Instruction::SDiv;
        break;

    case op_lshift:
        instr = Instruction::Shl;
        break;

    case op_rshift:
        instr = Instruction::LShr;
        break;

    case op_urshift:
        instr = Instruction::AShr;
        break;

    case op_bitand:
        instr = Instruction::And;
        break;

    case op_bitxor:
        instr = Instruction::Xor;
        break;

    case op_bitor:
        instr = Instruction::Or;
        break;

    case op_mod:
        instr = Instruction::SRem;
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE option=" << option << std::endl;
        return NULL;
    }

    llvm::Value *int32Left = CodeGenJSValue32FunctionToInt32(context, jsValueLeft);
    llvm::Value *int32Right = CodeGenJSValue32FunctionToInt32(context, jsValueRight);

    if ((NULL != int32Left) && (NULL != int32Right))
    {
    	return BinaryOperator::Create(instr, int32Left, int32Right, "", context.currentBlock());
    }

    return NULL;
}

static llvm::Value *CodeGenJSValue32FunctionOptionForJSValue(CodeGenContext& context, llvm::Value *jsValueLeftRef, llvm::Value *jsValueRightRef, OpcodeID option)
{
    const char *functionName = NULL;
    switch (option)
    {
    case op_plusplus:
        functionName = "_ejs_op_plusplus";
        break;

    case op_minusminus:
        functionName = "_ejs_op_minusminus";
        break;

    case op_add:
        functionName = "_ejs_op_add";
        break;

    case op_sub:
        functionName = "_ejs_op_sub";
        break;

    case op_mul:
        functionName = "_ejs_op_mult";
        break;

    case op_div:
        functionName = "_ejs_op_div";
        break;

    case op_lshift:
        functionName = "_ejs_op_lsh";
        break;

    case op_rshift:
        functionName = "_ejs_op_rsh";
        break;

    case op_urshift:
        functionName = "_ejs_op_ursh";
        break;

    case op_bitand:
        functionName = "_ejs_op_bitwise_and";
        break;

    case op_bitxor:
        functionName = "_ejs_op_bitwise_xor";
        break;

    case op_bitor:
        functionName = "_ejs_op_bitwise_or";
        break;

    case op_mod:
        functionName = "_ejs_op_mod";
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE option=" << option << std::endl;
        return NULL;
    }

	llvm::Function *function = context.getFunction(functionName);
    if (NULL == function)
    {
        std::cout <<  __FUNCTION__ << " Can't find " << functionName << "()" << std::endl;
        return NULL;
    }
	
	std::vector<llvm::Value*> args;
	args.push_back(jsValueLeftRef);
	args.push_back(jsValueRightRef);
	CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    call->setCallingConv(CallingConv::C);
    call->setTailCall(false);

    if ((op_plusplus == option) || (op_minusminus == option))
    {
        AttributeSet call_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;
        
        AttrBuilder B1;
        B1.addAttribute(Attribute::ByVal);
        B1.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B1);
        Attrs.push_back(PAS);

        AttrBuilder B2;
        B2.addAttribute(Attribute::ZExt);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 0U, B2);
        Attrs.push_back(PAS);
        
        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        call->setAttributes(call_PAL);
    }
    else
    {
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

llvm::Value *CodeGenJSValue32FunctionOption(CodeGenContext& context, llvm::Value *jsValueLeft, llvm::Value *jsValueRight, OpcodeID option)
{
    if ((NULL == jsValueLeft) || (NULL == jsValueRight))
    {
        std::cout <<  __FUNCTION__ << " BADPARAM" << std::endl;
        return NULL;
    }

    llvm::Type *typeRight = jsValueRight->getType();
    llvm::Type *typeLeft = jsValueLeft->getType();
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    int bitWidth;
    // Check both type:
    // if left is double or int32, change right the same as it
    // or, change both to (union.ejsval *)
    switch (typeLeft->getTypeID())
    {
    case llvm::Type::DoubleTyID:
        return CodeGenJSValue32FunctionOptionForDouble(context, jsValueLeft, jsValueRight, option);

    case llvm::Type::IntegerTyID:
        bitWidth = cast<llvm::IntegerType>(typeLeft)->getBitWidth();
        if (64 == bitWidth)
        {
            llvm::Value *jsValueLeftRef = CodeGenJSValue32Construct(context, "jsValueLeftRef");
            CodeGenJSValue32FunctionCopy(context, jsValueLeftRef, jsValueLeft);
            return CodeGenJSValue32FunctionOption(context, jsValueLeftRef, jsValueRight, option);
        }
        else if (32 == bitWidth)
        {
            return CodeGenJSValue32FunctionOptionForInt32(context, jsValueLeft, jsValueRight, option);
        }
        else if (1 == bitWidth)
        {
            llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
            RegisterID *int32Result = new ZExtInst(jsValueLeft, int32Type, "", context.currentBlock());
            return CodeGenJSValue32FunctionOptionForInt32(context, int32Result, jsValueRight, option);
        }
        else
        {
            std::cout <<  __FUNCTION__ << " BAD IntegerTyID OF jsValueLeft" << std::endl;
            return NULL;
        }
        break;

    case llvm::Type::StructTyID:
        if (jsValueType == typeLeft)
        {
            // change jsValueLeft from (union.ejsval) to (union.ejsval *)
            llvm::Value *jsValueLeftRef = CodeGenJSValue32Construct(context, "jsValueLeftRef");
            CodeGenJSValue32FunctionCopy(context, jsValueLeftRef, jsValueLeft);
            return CodeGenJSValue32FunctionOption(context, jsValueLeftRef, jsValueRight, option);
        }
        else
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE OF jsValueLeft" << std::endl;
            return NULL;
        }
        break;

    case llvm::Type::PointerTyID:
        if (jsValuePtrType != typeLeft)
        {
            jsValueLeft = new llvm::LoadInst(jsValueLeft, "", false, context.currentBlock());;
            return CodeGenJSValue32FunctionOption(context, jsValueLeft, jsValueRight, option);
        }
        else if ((op_plusplus == option) || (op_minusminus == option))
        {
            // jsValueRight should be bool type
        }
        else if (jsValuePtrType != typeRight)
        {
            // change jsValueLeft to (union.ejsval *)
            llvm::Value *jsValueRightRef = CodeGenJSValue32Construct(context, "jsValueRightRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRightRef, jsValueRight);
            jsValueRight = jsValueRightRef;
        }
        return CodeGenJSValue32FunctionOptionForJSValue(context, jsValueLeft, jsValueRight, option);
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE OF jsValueLeft" << std::endl;
        return NULL;
    }

    return NULL;
}

/*
 deal with >, >=, <, <=, ==, !=, so just cast to double type if needed
*/
static llvm::Value *CodeGenJSValue32FunctionCmpForDouble(CodeGenContext& context, llvm::Value *jsValueLeft, llvm::Value *jsValueRight, OpcodeID option)
{
    if ((NULL == jsValueLeft) || (NULL == jsValueRight))
    {
        std::cout <<  __FUNCTION__ << " BADPARAM" << std::endl;
        return NULL;
    }

	unsigned short instrCmp;
    switch (option)
    {
    case op_eq:
        instrCmp = ICmpInst::FCMP_OEQ;
        break;

    case op_neq:
        instrCmp = ICmpInst::FCMP_ONE;
        break;

    case op_less:
        instrCmp = ICmpInst::FCMP_OLT;
        break;

    case op_lesseq:
        instrCmp = ICmpInst::FCMP_OLE;
        break;

    case op_greater:
        instrCmp = ICmpInst::FCMP_OGT;
        break;

    case op_greatereq:
        instrCmp = ICmpInst::FCMP_OGE;
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE option=" << option << std::endl;
        return NULL;
    }

    llvm::Value *doubleLeft = CodeGenJSValue32FunctionToDouble(context, jsValueLeft);
    llvm::Value *doubleRight = CodeGenJSValue32FunctionToDouble(context, jsValueRight);

    if ((NULL != doubleLeft) && (NULL != doubleRight))
    {
        return CmpInst::Create(Instruction::FCmp, instrCmp, doubleLeft, doubleRight, "", context.currentBlock());
    }

    return NULL;
}

static llvm::Value *CodeGenJSValue32FunctionCmpForInt32(CodeGenContext& context, llvm::Value *jsValueLeft, llvm::Value *jsValueRight, OpcodeID option)
{
    if ((NULL == jsValueLeft) || (NULL == jsValueRight))
    {
        std::cout <<  __FUNCTION__ << " BADPARAM" << std::endl;
        return NULL;
    }

	unsigned short instrCmp;
    switch (option)
    {
    case op_eq:
        instrCmp = ICmpInst::ICMP_EQ;
        break;

    case op_neq:
        instrCmp = ICmpInst::ICMP_NE;
        break;

    case op_less:
        instrCmp = ICmpInst::ICMP_SLT;
        break;

    case op_lesseq:
        instrCmp = ICmpInst::ICMP_SLE;
        break;

    case op_greater:
        instrCmp = ICmpInst::ICMP_SGT;
        break;

    case op_greatereq:
        instrCmp = ICmpInst::ICMP_SGE;
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE option=" << option << std::endl;
        return NULL;
    }

    llvm::Value *int32Left = CodeGenJSValue32FunctionToInt32(context, jsValueLeft);
    llvm::Value *int32Right = CodeGenJSValue32FunctionToInt32(context, jsValueRight);

    if ((NULL != int32Left) && (NULL != int32Right))
    {
        return CmpInst::Create(Instruction::ICmp, instrCmp, int32Left, int32Right, "", context.currentBlock());
    }

    return NULL;
}

static llvm::Value *CodeGenJSValue32FunctionCmpForJSValue(CodeGenContext& context, llvm::Value *jsValueLeftRef, llvm::Value *jsValueRightRef, OpcodeID option)
{
    const char *functionName = NULL;
    switch (option)
    {
    case op_eq:
        if (jsValueLeftRef == jsValueRightRef)
        {
            return llvm::ConstantInt::getTrue(llvm::getGlobalContext());
        }
        functionName = "_ejs_op_eq";
        break;

    case op_neq:
        if (jsValueLeftRef == jsValueRightRef)
        {
            return llvm::ConstantInt::getFalse(llvm::getGlobalContext());
        }
        functionName = "_ejs_op_neq";
        break;

    case op_stricteq:
        if (jsValueLeftRef == jsValueRightRef)
        {
            return llvm::ConstantInt::getTrue(llvm::getGlobalContext());
        }
        functionName = "_ejs_op_strict_eq";
        break;

    case op_nstricteq:
        if (jsValueLeftRef == jsValueRightRef)
        {
            return llvm::ConstantInt::getFalse(llvm::getGlobalContext());
        }
        functionName = "_ejs_op_strict_neq";
        break;

    case op_less:
        functionName = "_ejs_op_lt";
        break;

    case op_lesseq:
        functionName = "_ejs_op_le";
        break;

    case op_greater:
        functionName = "_ejs_op_gt";
        break;

    case op_greatereq:
        functionName = "_ejs_op_ge";
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE option=" << option << std::endl;
        return NULL;
    }

	llvm::Function *function = context.getFunction(functionName);
    if (NULL == function)
    {
        std::cout <<  __FUNCTION__ << " Can't find " << functionName << "()" << std::endl;
        return NULL;
    }
	
	std::vector<llvm::Value*> args;
	args.push_back(jsValueLeftRef);
	args.push_back(jsValueRightRef);
	CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
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
        
        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        call->setAttributes(call_PAL);
    }

    return CodeGenJSValue32FunctionIsTrue(context, call);
}

llvm::Value *CodeGenJSValue32FunctionCmp(CodeGenContext& context, llvm::Value *jsValueLeft, llvm::Value *jsValueRight, OpcodeID option)
{
    if ((NULL == jsValueLeft) || (NULL == jsValueRight))
    {
        std::cout <<  __FUNCTION__ << " BADPARAM" << std::endl;
        return NULL;
    }

    llvm::Type *typeRight = jsValueRight->getType();
    llvm::Type *typeLeft = jsValueLeft->getType();
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    int bitWidth;
    // Check both type:
    // if left is double or int32, change right the same as it
    // or, change both to (union.ejsval *)
    switch (typeLeft->getTypeID())
    {
    case llvm::Type::DoubleTyID:
        return CodeGenJSValue32FunctionCmpForDouble(context, jsValueLeft, jsValueRight, option);

    case llvm::Type::IntegerTyID:
        bitWidth = cast<llvm::IntegerType>(typeLeft)->getBitWidth();
        if (64 == bitWidth)
        {
            llvm::Value *jsValueLeftRef = CodeGenJSValue32Construct(context, "jsValueLeftRef");
            CodeGenJSValue32FunctionCopy(context, jsValueLeftRef, jsValueLeft);
            return CodeGenJSValue32FunctionCmp(context, jsValueLeftRef, jsValueRight, option);
        }
        else if (32 == bitWidth)
        {
            return CodeGenJSValue32FunctionCmpForInt32(context, jsValueLeft, jsValueRight, option);
        }
        else if (1 == bitWidth)
        {
            llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
            RegisterID *int32Result = new ZExtInst(jsValueLeft, int32Type, "", context.currentBlock());
            return CodeGenJSValue32FunctionCmpForInt32(context, int32Result, jsValueRight, option);
        }
        else
        {
            std::cout <<  __FUNCTION__ << " BAD IntegerTyID OF jsValueLeft" << std::endl;
            return NULL;
        }
        break;

    case llvm::Type::StructTyID:
        if (jsValueType == typeLeft)
        {
            // change jsValueLeft from (union.ejsval) to (union.ejsval *)
            llvm::Value *jsValueLeftRef = CodeGenJSValue32Construct(context, "jsValueLeftRef");
            CodeGenJSValue32FunctionCopy(context, jsValueLeftRef, jsValueLeft);
            return CodeGenJSValue32FunctionCmp(context, jsValueLeftRef, jsValueRight, option);
        }
        else
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE OF jsValueLeft" << std::endl;
            return NULL;
        }
        break;

    case llvm::Type::PointerTyID:
        if (jsValuePtrType != typeLeft)
        {
            jsValueLeft = new llvm::LoadInst(jsValueLeft, "", false, context.currentBlock());;
            return CodeGenJSValue32FunctionCmp(context, jsValueLeft, jsValueRight, option);
        }
        else if (jsValuePtrType != typeRight)
        {
            // change jsValueRight to (union.ejsval *)
            llvm::Value *jsValueRightRef = CodeGenJSValue32Construct(context, "jsValueRightRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRightRef, jsValueRight);
            jsValueRight = jsValueRightRef;
            typeRight = jsValueRight->getType();
        }
        return CodeGenJSValue32FunctionCmpForJSValue(context, jsValueLeft, jsValueRight, option);
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE OF jsValueLeft" << std::endl;
        return NULL;
    }

    return NULL;
}

/*************************************************************
* PUBLIC FUNCTIONS                                                   *
**************************************************************/
llvm::Value *CodeGenJSValue32Construct(CodeGenContext& context, const char *name)
{
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    //jsValueRef = (ejsval *)alloca(sizeof(ejsval));
    llvm::AllocaInst *jsValueRef = new llvm::AllocaInst(jsValueType, name, context.currentBlock());
    jsValueRef->setAlignment(8);

    return jsValueRef;
}

/*
inline ejsval::ejsval(JSNullTag)
{
    u.asBits.tag = EJSVAL_TAG_NULL;
    u.asBits.payload = 0;
}
*/
llvm::Value *CodeGenJSValue32ConstructForNULL(CodeGenContext& context, const char *name)
{
    llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, name);
#if 0
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());

    CodeGenJSValue32FunctionCopyForOthers(context, jsValueRef, llvm::ConstantInt::get(int32Type, EJSVAL_TAG_NULL), llvm::ConstantInt::get(int32Type, 0));
#else
    llvm::Type *int64Type = llvm::Type::getInt64Ty(llvm::getGlobalContext());

    CodeGenJSValue32FunctionCopyForInt64(context, jsValueRef, llvm::ConstantInt::get(int64Type, ((uint64_t)EJSVAL_TAG_NULL)<<32));
#endif
    return jsValueRef;
}

/*
    inline ejsval::ejsval(JSUndefinedTag)
    {
        u.asBits.tag = EJSVAL_TAG_UNDEFINED;
        u.asBits.payload = 0;
    }
*/
llvm::Value *CodeGenJSValue32ConstructForUndefined(CodeGenContext& context, const char *name)
{
    llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, name);
#if 0
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());

    CodeGenJSValue32FunctionCopyForOthers(context, jsValueRef, llvm::ConstantInt::get(int32Type, EJSVAL_TAG_UNDEFINED), llvm::ConstantInt::get(int32Type, 0));
#else
    llvm::Type *int64Type = llvm::Type::getInt64Ty(llvm::getGlobalContext());

    CodeGenJSValue32FunctionCopyForInt64(context, jsValueRef, llvm::ConstantInt::get(int64Type, ((uint64_t)EJSVAL_TAG_UNDEFINED)<<32));
#endif
    return jsValueRef;
}

/*
    inline ejsval::ejsval(JSTrueTag)
    {
        u.asBits.tag = EJSVAL_TAG_BOOLEAN;
        u.asBits.payload = 1;
    }
*/
llvm::Value *CodeGenJSValue32ConstructForBool(CodeGenContext& context, bool value, const char *name)
{
    llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, name);
    int intValue = value ? 1 : 0;
#if 0
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());

    CodeGenJSValue32FunctionCopyForOthers(context, jsValueRef, llvm::ConstantInt::get(int32Type, EJSVAL_TAG_BOOLEAN), llvm::ConstantInt::get(int32Type, intValue));
#else
    llvm::Type *int64Type = llvm::Type::getInt64Ty(llvm::getGlobalContext());

    CodeGenJSValue32FunctionCopyForInt64(context, jsValueRef, llvm::ConstantInt::get(int64Type, ((uint64_t)EJSVAL_TAG_BOOLEAN)<<32 | (uint64_t)intValue));
#endif
    return jsValueRef;
}

llvm::Value *CodeGenJSValue32ConstructForBool(CodeGenContext& context, llvm::Value *value, const char *name)
{
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, name);

    CodeGenJSValue32FunctionCopyForOthers(context, jsValueRef, llvm::ConstantInt::get(int32Type, EJSVAL_TAG_BOOLEAN), value);
    return jsValueRef;
}

/*
    inline ejsval::ejsval(int i)
    {
        u.asBits.tag = EJSVAL_TAG_INT32;
        u.asBits.payload = i;
    }
*/
llvm::Value *CodeGenJSValue32ConstructForInt32(CodeGenContext& context, int value, const char *name)
{
    llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, name);
#if 0
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());

    CodeGenJSValue32FunctionCopyForOthers(context, jsValueRef, llvm::ConstantInt::get(int32Type, EJSVAL_TAG_INT32), llvm::ConstantInt::get(int32Type, value));
#else
    llvm::Type *int64Type = llvm::Type::getInt64Ty(llvm::getGlobalContext());

    CodeGenJSValue32FunctionCopyForInt64(context, jsValueRef, llvm::ConstantInt::get(int64Type, ((uint64_t)EJSVAL_TAG_INT32)<<32 | (uint64_t)value));
#endif
    return jsValueRef;
}

/*
    ejsval::ejsval(EncodeAsDoubleTag, double d)
    {
        u.asDouble = d;
    }
*/
llvm::Value *CodeGenJSValue32ConstructForDouble(CodeGenContext& context, double value, const char *name)
{
    llvm::Type *doubleType = llvm::Type::getDoubleTy(llvm::getGlobalContext());
    llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, name);

    CodeGenJSValue32FunctionCopyForDouble(context, jsValueRef, llvm::ConstantFP::get(doubleType, value));
    return jsValueRef;
}

/*
    inline ejsval::ejsval(JSCell* ptr)
    {
        u.asBits.tag = EJSVAL_TAG_OBJECT;
        u.asBits.payload = reinterpret_cast<int32_t>(ptr);
    }
*/
llvm::Value *CodeGenJSValue32ConstructForObject(CodeGenContext& context, llvm::Value *jsObjRef, const char *name)
{
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, name);

    llvm::Value *jsObjAddr = new llvm::PtrToIntInst(jsObjRef, int32Type, "", context.currentBlock());
    CodeGenJSValue32FunctionCopyForOthers(context, jsValueRef, llvm::ConstantInt::get(int32Type, EJSVAL_TAG_OBJECT), jsObjAddr);
    return jsValueRef;
}

llvm::Value *CodeGenJSValue32ConstructIfNeeded(CodeGenContext& context, llvm::Value *jsValue)
{
    llvm::Type *type = jsValue->getType();
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);

    if (type != jsValuePtrType)
    {
        RegisterID *jsTmpRef = CodeGenJSValue32Construct(context, "jsTmpRef");
        CodeGenJSValue32FunctionCopy(context, jsTmpRef, jsValue);
        return jsTmpRef;
    }

    return jsValue;
}

llvm::Value *CodeGenJSValue32Destruct(CodeGenContext& context, llvm::Value *jsValue)
{
    return NULL;
}

/*
    inline bool ejsval::isNull() const
    {
        return u.asBits.tag == EJSVAL_TAG_NULL;
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsNULL(CodeGenContext& context, llvm::Value *jsValue)
{
    llvm::Type *int32PtrType = llvm::Type::getInt32PtrTy(llvm::getGlobalContext());
    //intRef = (int *)jsValueRef;
	llvm::Value *intRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValue, int32PtrType, "asIntPtr", context.currentBlock());
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());

	//tagRef = intRef[jsValue32TagPart]
    llvm::Value *tagRef = llvm::GetElementPtrInst::Create(intRef, context.getVariable("jsValue32TagPart"), "tagRef", context.currentBlock());
    LoadInst* tag = new llvm::LoadInst(tagRef, "tag", false, context.currentBlock());
    tag->setAlignment(4);

    //return (tag == EJSVAL_TAG_NULL)
    return llvm::CmpInst::Create(llvm::CmpInst::Instruction::ICmp, ICmpInst::ICMP_EQ, ConstantInt::get(int32Type, EJSVAL_TAG_NULL), tag, "", context.currentBlock());
}

/*
    inline bool ejsval::isNull() const
    {
        return u.asBits.tag == EJSVAL_TAG_UNDEFINED;
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsUndefined(CodeGenContext& context, llvm::Value *jsValue)
{
    if (NULL == jsValue)
    {
        std::cout <<  __FUNCTION__ << " BADPARAM" << std::endl;
        return NULL;
    }

    llvm::Type *type = jsValue->getType();
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    llvm::Type *int64Type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
    if ((type == jsValueType) || (type == int64Type))
    {
    	llvm::Value *valueRef = CodeGenJSValue32Construct(context, "");
        CodeGenJSValue32FunctionCopy(context, valueRef, jsValue);
    
        return CodeGenJSValue32FunctionIsUndefined(context, valueRef);
    }
    else if (type == jsValuePtrType)
    {
        llvm::Type *int32PtrType = llvm::Type::getInt32PtrTy(llvm::getGlobalContext());
        //intRef = (int *)jsValueRef;
    	llvm::Value *intRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValue, int32PtrType, "asIntPtr", context.currentBlock());
        llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    
    	//tagRef = intRef[jsValue32TagPart]
        llvm::Value *tagRef = llvm::GetElementPtrInst::Create(intRef, context.getVariable("jsValue32TagPart"), "tagRef", context.currentBlock());
        LoadInst* tag = new llvm::LoadInst(tagRef, "tag", false, context.currentBlock());
        tag->setAlignment(4);
    
        //return (tag == EJSVAL_TAG_UNDEFINED)
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::ICmpInst::ICMP_EQ, llvm::ConstantInt::get(int32Type, EJSVAL_TAG_UNDEFINED), tag, "", context.currentBlock());
    }

    return llvm::ConstantInt::getFalse(llvm::getGlobalContext());
}
/*
    inline bool ejsval::isUndefinedOrNull() const
    {
        return isUndefined() || isNull();
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsUndefinedorNULL(CodeGenContext& context, llvm::Value *jsValue)
{
    return NULL;
}

/*
    inline bool ejsval::isTrue() const
    {
        return tag() == EJSVAL_TAG_BOOLEAN && payload();
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsTrue(CodeGenContext& context, llvm::Value *jsValue)
{
    if (NULL == jsValue)
    {
        std::cout <<  __FUNCTION__ << " BADPARAM" << std::endl;
        return NULL;
    }

    llvm::Type *type = jsValue->getType();
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Type *doubleType = llvm::Type::getDoubleTy(llvm::getGlobalContext());
    int bitWidth;
    // Check both type:
    // if left is double or int32, change right the same as it
    // or, change both to (union.ejsval *)
    switch (type->getTypeID())
    {
    case llvm::Type::DoubleTyID:
        return CodeGenJSValue32FunctionCmpForDouble(context, jsValue, ConstantFP::get(doubleType, 0.0), op_neq);

    case llvm::Type::IntegerTyID:
        bitWidth = cast<llvm::IntegerType>(type)->getBitWidth();
        if (64 == bitWidth)
        {
            llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
            return CodeGenJSValue32FunctionIsTrue(context, jsValueRef);
        }
        else if (32 == bitWidth)
        {
            return CodeGenJSValue32FunctionCmpForInt32(context, jsValue, ConstantInt::get(int32Type, 0, true), op_neq);
        }
        else if (1 == bitWidth)
        {
            return jsValue;
        }
        else
        {
            std::cout <<  __FUNCTION__ << " BAD IntegerTyID OF jsValueLeft" << std::endl;
            return NULL;
        }
        break;

    case llvm::Type::StructTyID:
        if (jsValueType == type)
        {
            // change jsValueLeft from (union.ejsval) to (union.ejsval *)
            llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
            return CodeGenJSValue32FunctionIsTrue(context, jsValueRef);
        }
        else
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE OF jsValue" << std::endl;
            return NULL;
        }
        break;

    case llvm::Type::PointerTyID:
        if (jsValuePtrType != type)
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE OF jsValue" << std::endl;
            return NULL;
        }
        else
        {
        	llvm::Value *result = CodeGenJSValue32FunctionToBool(context, jsValue);

            return CodeGenJSValue32FunctionIsTrue(context, result);
        }
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE OF jsValue" << std::endl;
        return NULL;
    }

    return NULL;
}

/*
    inline bool ejsval::isFalse() const
    {
        return tag() == EJSVAL_TAG_BOOLEAN && !payload();
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsFalse(CodeGenContext& context, llvm::Value *jsValue)
{
    if (NULL == jsValue)
    {
        std::cout <<  __FUNCTION__ << " BADPARAM" << std::endl;
        return NULL;
    }

    if (CmpInst::classof(jsValue))
    {
        return jsValue;
    }

    llvm::Type *type = jsValue->getType();
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Type *doubleType = llvm::Type::getDoubleTy(llvm::getGlobalContext());
    int bitWidth;
    // Check both type:
    // if left is double or int32, change right the same as it
    // or, change both to (union.ejsval *)
    switch (type->getTypeID())
    {
    case llvm::Type::DoubleTyID:
        return CodeGenJSValue32FunctionCmpForDouble(context, jsValue, ConstantFP::get(doubleType, 0.0), op_eq);

    case llvm::Type::IntegerTyID:
        bitWidth = cast<llvm::IntegerType>(type)->getBitWidth();
        if (64 == bitWidth)
        {
            llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
            return CodeGenJSValue32FunctionIsTrue(context, jsValueRef);
        }
        else if (32 == bitWidth)
        {
            return CodeGenJSValue32FunctionCmpForInt32(context, jsValue, ConstantInt::get(int32Type, 0, true), op_eq);
        }
        else if (1 == bitWidth)
        {
            llvm::Value *trueV = llvm::ConstantInt::getTrue(llvm::getGlobalContext());
            return BinaryOperator::Create(Instruction::Xor, jsValue, trueV, "lnot", context.currentBlock());
        }
        else
        {
            std::cout <<  __FUNCTION__ << " BAD IntegerTyID OF jsValueLeft" << std::endl;
            return NULL;
        }
        break;

    case llvm::Type::StructTyID:
        if (jsValueType == type)
        {
            // change jsValueLeft from (union.ejsval) to (union.ejsval *)
            llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
            return CodeGenJSValue32FunctionIsTrue(context, jsValueRef);
        }
        else
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE OF jsValue" << std::endl;
            return NULL;
        }
        break;

    case llvm::Type::PointerTyID:
        if (jsValuePtrType != type)
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE OF jsValue" << std::endl;
            return NULL;
        }
        else
        {
        	llvm::Value *result = CodeGenJSValue32FunctionToBool(context, jsValue);
        
            return CodeGenJSValue32FunctionIsFalse(context, result);
        }
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE OF jsValue" << std::endl;
        return NULL;
    }

    return NULL;
}

/*
    inline bool ejsval::isBoolean() const
    {
        return tag() == EJSVAL_TAG_BOOLEAN;
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsBool(CodeGenContext& context, llvm::Value *jsValue)
{
    llvm::Type *int32PtrType = llvm::Type::getInt32PtrTy(llvm::getGlobalContext());
    //intRef = (int *)jsValueRef;
	llvm::Value *intRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValue, int32PtrType, "asIntPtr", context.currentBlock());
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());

	//tagRef = intRef[jsValue32TagPart]
    llvm::Value *tagRef = llvm::GetElementPtrInst::Create(intRef, context.getVariable("jsValue32TagPart"), "tagRef", context.currentBlock());
    LoadInst* tag = new llvm::LoadInst(tagRef, "tag", false, context.currentBlock());
    tag->setAlignment(4);

    //return (tag == EJSVAL_TAG_BOOLEAN)
    return llvm::CmpInst::Create(llvm::CmpInst::Instruction::ICmp, llvm::ICmpInst::ICMP_EQ, llvm::ConstantInt::get(int32Type, EJSVAL_TAG_BOOLEAN), tag, "", context.currentBlock());
}

/*
    inline bool ejsval::isCell() const
    {
        return tag() == EJSVAL_TAG_OBJECT;
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsObject(CodeGenContext& context, llvm::Value *jsValue)
{
    llvm::Type *int32PtrType = llvm::Type::getInt32PtrTy(llvm::getGlobalContext());
    //intRef = (int *)jsValueRef;
	llvm::Value *intRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValue, int32PtrType, "asIntPtr", context.currentBlock());
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());

	//tagRef = intRef[jsValue32TagPart]
    llvm::Value *tagRef = llvm::GetElementPtrInst::Create(intRef, context.getVariable("jsValue32TagPart"), "tagRef", context.currentBlock());
    LoadInst* tag = new llvm::LoadInst(tagRef, "tag", false, context.currentBlock());
    tag->setAlignment(4);

    //return (tag == EJSVAL_TAG_OBJECT)
    return llvm::CmpInst::Create(llvm::Instruction::ICmp, ICmpInst::ICMP_EQ, ConstantInt::get(int32Type, EJSVAL_TAG_OBJECT), tag, "", context.currentBlock());
}

/*
    inline bool ejsval::isDouble() const
    {
        return tag() < LowestTag;
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsDouble(CodeGenContext& context, llvm::Value *jsValue)
{
    llvm::Type *int32PtrType = llvm::Type::getInt32PtrTy(llvm::getGlobalContext());
    //intRef = (int *)jsValueRef;
	llvm::Value *intRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValue, int32PtrType, "asIntPtr", context.currentBlock());
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());

	//tagRef = intRef[jsValue32TagPart]
    llvm::Value *tagRef = llvm::GetElementPtrInst::Create(intRef, context.getVariable("jsValue32TagPart"), "tagRef", context.currentBlock());
    LoadInst* tag = new llvm::LoadInst(tagRef, "tag", false, context.currentBlock());
    tag->setAlignment(4);

    //return (tag < LowestTag)
    return llvm::CmpInst::Create(llvm::Instruction::ICmp, ICmpInst::ICMP_SGT, ConstantInt::get(int32Type, EJSVAL_TAG_OBJECT), tag, "", context.currentBlock());
}

/*
    inline bool ejsval::isInt32() const
    {
        return tag() == EJSVAL_TAG_INT32;
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsInt32(CodeGenContext& context, llvm::Value *jsValue)
{
    llvm::Type *int32PtrType = llvm::Type::getInt32PtrTy(llvm::getGlobalContext());
    //intRef = (int *)jsValueRef;
	llvm::Value *intRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValue, int32PtrType, "asIntPtr", context.currentBlock());
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());

	//tagRef = intRef[jsValue32TagPart]
    llvm::Value *tagRef = llvm::GetElementPtrInst::Create(intRef, context.getVariable("jsValue32TagPart"), "tagRef", context.currentBlock());
    LoadInst* tag = new llvm::LoadInst(tagRef, "tag", false, context.currentBlock());
    tag->setAlignment(4);

    //return (tag == EJSVAL_TAG_INT32)
    return llvm::CmpInst::Create(llvm::CmpInst::Instruction::ICmp, ICmpInst::ICMP_EQ, ConstantInt::get(int32Type, EJSVAL_TAG_INT32), tag, "", context.currentBlock());
}

/*
    inline bool ejsval::isUInt32() const
    {
        return isInt32() && asInt32() >= 0;
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsUInt32(CodeGenContext& context, llvm::Value *jsValue)
{
    return NULL;
}

/*
    inline bool ejsval::isNumber() const
    {
        return isInt32() || isDouble();
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsNumber(CodeGenContext& context, llvm::Value *jsValue)
{
    return NULL;
}

/*
    inline bool ejsval::isString() const
    {
        return isCell() && asCell()->isString();
    }
*/
llvm::Value *CodeGenJSValue32FunctionIsString(CodeGenContext& context, llvm::Value *jsValue)
{
    return NULL;
}

/*
    inline bool ejsval::toBoolean(ExecState* exec) const
    {
        if (isInt32())
            return asInt32() != 0;
        if (isDouble())
            return asDouble() > 0.0 || asDouble() < 0.0; // false for NaN
        if (isCell())
            return asCell()->toBoolean(exec);
        return isTrue(); // false, null, and undefined all convert to false.
    }
*/
llvm::Value *CodeGenJSValue32FunctionToBool(CodeGenContext& context, llvm::Value *jsValue)
{
	llvm::Function *function = context.getFunction("ToEJSBool");
    if (NULL == function)
    {
        std::cout <<  __FUNCTION__ << " Can't find ToEJSBool()" << std::endl;
        return NULL;
    }
	
	std::vector<llvm::Value*> args;
	args.push_back(jsValue);
	CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
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

/*
*/
llvm::Value *CodeGenJSValue32FunctionToObject(CodeGenContext& context, llvm::Value *jsValue)
{
    return NULL;
}

/*
    ALWAYS_INLINE double ejsval::toNumber(ExecState* exec) const
    {
        if (isInt32())
            return u.asBits.payload;
        if (isDouble())
            return u.asDouble;
        if (isCell())
            return asCell()->toNumber(exec);
        if (isTrue())
            return 1.0;
        return isUndefined() ? std::numeric_limits<double>::quiet_NaN() : 0; // null and false both convert to 0.
    }
*/
llvm::Value *CodeGenJSValue32FunctionToDouble(CodeGenContext& context, llvm::Value *jsValue)
{
    llvm::Type *type = jsValue->getType();
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    llvm::Type *doubleType = llvm::Type::getDoubleTy(llvm::getGlobalContext());
    int bitWidth;

    switch (type->getTypeID())
    {
    case llvm::Type::DoubleTyID:
        return jsValue;
        break;

    case llvm::Type::IntegerTyID:
        bitWidth = cast<llvm::IntegerType>(type)->getBitWidth();
        if (64 == bitWidth)
        {
            llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
            return CodeGenJSValue32FunctionToDouble(context, jsValueRef);
        }
        else if (32 == bitWidth)
        {
            return new SIToFPInst(jsValue, doubleType, "conv", context.currentBlock());
        }
        else if (1 == bitWidth)
        {
            llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
            RegisterID *int32Result = new ZExtInst(jsValue, int32Type, "", context.currentBlock());
            return new SIToFPInst(int32Result, doubleType, "conv", context.currentBlock());
        }
        else
        {
            std::cout <<  __FUNCTION__ << " BAD IntegerTyID OF jsValue" << std::endl;
            return NULL;
        }
        break;

    case llvm::Type::StructTyID:
        if (jsValueType == type)
        {
            llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
            return CodeGenJSValue32FunctionToDouble(context, jsValueRef);
        }
        else
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE OF jsValue" << std::endl;
        }
        break;

    case llvm::Type::PointerTyID:
        if (jsValuePtrType == type)
        {
        	llvm::Function *function = context.getFunction("ToDouble");
        	if (function == NULL)
        	{
                std::cout <<  __FUNCTION__ << " Can't find ToDouble()" << std::endl;
        		context.abort();
        		return NULL;
        	}
        	
        	std::vector<llvm::Value*> args;
        	args.push_back(jsValue);
        	CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
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
        else
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE POINTER OF jsValue" << std::endl;
            return NULL;
        }
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE OF jsValue" << std::endl;
        break;
    }

    return NULL;
}

/*
    ALWAYS_INLINE int32_t ejsval::toInt32(ExecState* exec) const
    {
        if (isInt32())
            return u.asBits.payload;
        if (isDouble())
            return JSC::toInt32(u.asDouble);
        if (isCell())
            return asCell()->toNumber(exec);
        if (isTrue())
            return 1;
        return 0; // null and false both convert to 0.
    }
*/
llvm::Value *CodeGenJSValue32FunctionToInt32(CodeGenContext& context, llvm::Value *jsValue)
{
    llvm::Type *type = jsValue->getType();
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    int bitWidth;

    switch (type->getTypeID())
    {
    case llvm::Type::DoubleTyID:
        return new FPToSIInst(jsValue, int32Type, "conv", context.currentBlock());
        break;

    case llvm::Type::IntegerTyID:
        bitWidth = cast<llvm::IntegerType>(type)->getBitWidth();
        if (64 == bitWidth)
        {
            llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
            return CodeGenJSValue32FunctionToInt32(context, jsValueRef);
        }
        else if (32 == bitWidth)
        {
            return jsValue;
        }
        else if (1 == bitWidth)
        {
            return new ZExtInst(jsValue, int32Type, "", context.currentBlock());
        }
        else
        {
            std::cout <<  __FUNCTION__ << " BAD IntegerTyID OF jsValue" << std::endl;
            return NULL;
        }
        break;

    case llvm::Type::StructTyID:
        if (jsValueType == type)
        {
            llvm::Value *jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
            return CodeGenJSValue32FunctionToInt32(context, jsValueRef);
        }
        else
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE OF jsValue" << std::endl;
        }
        break;

    case llvm::Type::PointerTyID:
        if (jsValuePtrType == type)
        {
        	llvm::Function *function = context.getFunction("ToUint32");
        	if (function == NULL)
        	{
                std::cout <<  __FUNCTION__ << " Can't find ToUint32()" << std::endl;
        		context.abort();
        		return NULL;
        	}
        	
        	std::vector<llvm::Value*> args;
        	args.push_back(jsValue);
        	CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
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
        else
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE POINTER OF jsValue" << std::endl;
            return NULL;
        }
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE OF jsValue" << std::endl;
        break;
    }

    return NULL;
}

/**
 * Notice:
 * the function CodeGenJSValue32FunctionToInt64 is used for ABI return for union.ejsval actually
 * so,
 * change int32 to int64
 * change bool to int64
 * change double to union.ejsval, and return union.ejsval.asInt64
 * if it's a union.ejsval value, return union.ejsval.asInt64
 */
llvm::Value *CodeGenJSValue32FunctionToInt64(CodeGenContext& context, llvm::Value *jsValue)
{
    llvm::Type *type = jsValue->getType();
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    llvm::Value *jsValueRef;
    int bitWidth;
    switch (type->getTypeID())
    {
    case llvm::Type::DoubleTyID:
        jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
        CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
        return CodeGenJSValue32FunctionToInt64(context, jsValueRef);
        break;

    case llvm::Type::IntegerTyID:
        bitWidth = cast<llvm::IntegerType>(type)->getBitWidth();
        if (64 == bitWidth)
        {
            return jsValue;
        }
        else if (32 == bitWidth)
        {
            jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
            return CodeGenJSValue32FunctionToInt64(context, jsValueRef);
        }
        else if (1 == bitWidth)
        {
            jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
            CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
            return CodeGenJSValue32FunctionToInt64(context, jsValueRef);
        }
        else
        {
            std::cout <<  __FUNCTION__ << " BAD IntegerTyID OF jsValue" << std::endl;
            return NULL;
        }
        break;

    case llvm::Type::StructTyID:
        if (jsValueType != type)
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE OF jsValueLeft" << std::endl;
            return NULL;
        }
        jsValueRef = CodeGenJSValue32Construct(context, "jsValueRef");
        CodeGenJSValue32FunctionCopy(context, jsValueRef, jsValue);
        return CodeGenJSValue32FunctionToInt64(context, jsValueRef);
        break;

    case llvm::Type::PointerTyID:
        if (llvm::isa<llvm::Function>(jsValue))
        {
            llvm::Function *function = (llvm::Function *)jsValue;
            const std::string &name = function->getName();
            jsValueRef = CodeGenJSObjectMakeFunctionCall(context, function, name);
            return CodeGenJSValue32FunctionToInt64(context, jsValueRef);
        }
        else if (jsValuePtrType != type)
        {
            std::cout <<  __FUNCTION__ << " UNKNOWN STRUCTTYPE OF jsValue" << std::endl;
            return NULL;
        }
        else
        {
            llvm::Type *int64PtrType = llvm::Type::getInt64PtrTy(llvm::getGlobalContext());
            jsValueRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValue, int64PtrType, "asInt64Ptr", context.currentBlock());
            return new llvm::LoadInst(jsValueRef, "asInt64", false, context.currentBlock());
        }
        break;

    default:
        std::cout <<  __FUNCTION__ << " BADTYPE OF jsValue" << std::endl;
        return NULL;
    }

    return NULL;
}
/*
    inline UString ejsval::toString(ExecState* exec) const
    {
        if (isString())
            return static_cast<JSString*>(asCell())->value(exec);
        if (isInt32())
            return exec->globalData().numericStrings.add(asInt32());
        if (isDouble())
            return exec->globalData().numericStrings.add(asDouble());
        if (isTrue())
            return "true";
        if (isFalse())
            return "false";
        if (isNull())
            return "null";
        if (isUndefined())
            return "undefined";
        ASSERT(isCell());
        return asCell()->toString(exec);
    }
*/
llvm::Value *CodeGenJSValue32FunctionToString(CodeGenContext& context, llvm::Value *jsValue, llvm::Value *buffer, llvm::Value *bufferLength)
{
    const char *name = "JSValueToString";
	llvm::Function *function = context.getFunction(name);
	if (function == NULL)
	{
        llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
        llvm::Type *doubleType = llvm::Type::getDoubleTy(llvm::getGlobalContext());
        llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    	std::vector<llvm::Type*> argTypes;
		argTypes.push_back(jsValueType);

    	llvm::FunctionType *ftype = llvm::FunctionType::get(int32Type, makeArrayRef(argTypes), false);
    	function = llvm::Function::Create(ftype, llvm::GlobalValue::InternalLinkage, name, context.getModule());

    	llvm::BasicBlock *entryBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function, 0);
        llvm::BasicBlock *intcaseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "intcase", function, 0);
        llvm::BasicBlock *booleancaseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "booleancase", function, 0);
        llvm::BasicBlock* booleancasetrueBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "if.then",function,0);
        llvm::BasicBlock* booleancasefalseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "if.else",function,0);
        llvm::BasicBlock *nullcaseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "nullcase", function, 0);
        llvm::BasicBlock *undefinedcaseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "undefinedcase", function, 0);
        llvm::BasicBlock *objcaseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "objcase", function, 0);
        llvm::BasicBlock *doublecaseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "doublecase", function, 0);
        llvm::BasicBlock *returnBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "return", function, 0);

    	llvm::Function::arg_iterator argsValues = function->arg_begin();
        llvm::Value* argumentValue = argsValues++;
        llvm::Value* argumentBuffer = argsValues++;
        argumentBuffer->setName("buffer");
        llvm::Value* argumentLength = argsValues++;
        argumentLength->setName("bufferLength");
        
        // Block entry (entryBB)
        PointerType* PointerTy_15 = llvm::Type::getInt8PtrTy(llvm::getGlobalContext());
        AllocaInst* retvalRef = new AllocaInst(doubleType, "retval", entryBB);
        retvalRef->setAlignment(8);
        llvm::AllocaInst *jsValueRef = new llvm::AllocaInst(jsValueType, "jsValueRef", entryBB);
        jsValueRef->setAlignment(8);
        AllocaInst* bufferRef = new AllocaInst(PointerTy_15, "bufferRef", entryBB);
        bufferRef->setAlignment(4);
        AllocaInst* lengthRef = new AllocaInst(int32Type, "bufferLengthRef", entryBB);
        lengthRef->setAlignment(4);
        LoadInst* bufferValue = new LoadInst(bufferRef, "", false, booleancasetrueBB);
        bufferValue->setAlignment(4);
        LoadInst* lengthValue = new LoadInst(lengthRef, "", false, booleancasetrueBB);
        lengthValue->setAlignment(4);
        std::vector<Value*> snprintfParams;
        
        StoreInst* void_131 = new StoreInst(argumentBuffer, bufferRef, false, entryBB);
        void_131->setAlignment(4);
        StoreInst* void_132 = new StoreInst(argumentLength, lengthRef, false, entryBB);
        void_132->setAlignment(4);
        new llvm::StoreInst(argumentValue, jsValueRef, false, entryBB);
        
        llvm::Type *int32PtrType = llvm::Type::getInt32PtrTy(llvm::getGlobalContext());
        //intRef = (int *)jsValueRef;
    	llvm::Value *intRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValueRef, int32PtrType, "asIntPtr", entryBB);
    
    	//tagRef = intRef[jsValue32TagPart]
        llvm::GetElementPtrInst *tagRef = llvm::GetElementPtrInst::Create(intRef, context.getVariable("jsValue32TagPart"), "tagRef", entryBB);
        LoadInst* tag = new llvm::LoadInst(tagRef, "tag", false, entryBB);
        tag->setAlignment(4);
    	//payloadRef = intRef[jsValue32PayloadPart]
        llvm::GetElementPtrInst *payloadRef = llvm::GetElementPtrInst::Create(intRef, context.getVariable("jsValue32PayloadPart"), "payloadRef", entryBB);
        LoadInst* payload = new llvm::LoadInst(payloadRef, "payload", false, entryBB);
        payload->setAlignment(4);
    
        llvm::SwitchInst *switchInst = llvm::SwitchInst::Create(tag, doublecaseBB, 5, entryBB);
        switchInst->addCase(llvm::ConstantInt::get(int32Type, EJSVAL_TAG_INT32), intcaseBB);
        switchInst->addCase(llvm::ConstantInt::get(int32Type, EJSVAL_TAG_BOOLEAN), booleancaseBB);
        switchInst->addCase(llvm::ConstantInt::get(int32Type, EJSVAL_TAG_NULL), nullcaseBB);
        switchInst->addCase(llvm::ConstantInt::get(int32Type, EJSVAL_TAG_UNDEFINED), undefinedcaseBB);
        switchInst->addCase(llvm::ConstantInt::get(int32Type, EJSVAL_TAG_OBJECT), objcaseBB);
    
        // Block sw.bb (intcaseBB)
        LoadInst* ptr_137 = new llvm::LoadInst(bufferRef, "", false, intcaseBB);
        ptr_137->setAlignment(4);
        llvm::LoadInst* int32_138 = new llvm::LoadInst(lengthRef, "", false, intcaseBB);
        int32_138->setAlignment(4);
        llvm::LoadInst* int32_141 = new llvm::LoadInst(payload, "", false, intcaseBB);
        int32_141->setAlignment(4);
        snprintfParams.push_back(ptr_137);
        snprintfParams.push_back(int32_138);
        snprintfParams.push_back(context.getVariableRef("intFormat"));
        snprintfParams.push_back(int32_141);
        BranchInst::Create(returnBB, intcaseBB);
        
        // Block sw.bb3 (booleancaseBB)
        LoadInst* int32_143 = new LoadInst(payload, "", false, booleancaseBB);
        int32_143->setAlignment(4);
        ICmpInst* int1_cmp_144 = new ICmpInst(*booleancaseBB, ICmpInst::ICMP_NE, ConstantInt::get(int32Type, 0), int32_143, "cmp");
        BranchInst::Create(booleancasetrueBB, booleancasefalseBB, int1_cmp_144, booleancaseBB);
        
        // Block if.then (booleancasetrueBB)
        snprintfParams.push_back(bufferValue);
        snprintfParams.push_back(lengthValue);
        snprintfParams.push_back(context.getVariableRef("trueString"));
        BranchInst::Create(returnBB, booleancasetrueBB);
        
        // Block if.else (booleancasefalseBB)
        snprintfParams.push_back(bufferValue);
        snprintfParams.push_back(lengthValue);
        snprintfParams.push_back(context.getVariableRef("falseFormat"));
        BranchInst::Create(returnBB, booleancasefalseBB);
        
        // Block sw.bb8 (nullcaseBB)
        snprintfParams.push_back(bufferValue);
        snprintfParams.push_back(lengthValue);
        snprintfParams.push_back(context.getVariableRef("nullFormat"));
        BranchInst::Create(returnBB, nullcaseBB);
        
        // Block sw.bb10 (undefinedcaseBB)
        snprintfParams.push_back(bufferValue);
        snprintfParams.push_back(lengthValue);
        snprintfParams.push_back(context.getVariableRef("undefinedFormat"));
        BranchInst::Create(returnBB, undefinedcaseBB);
        
        // Block sw.bb12 (objcaseBB)
        snprintfParams.push_back(bufferValue);
        snprintfParams.push_back(lengthValue);
        snprintfParams.push_back(context.getVariableRef("objectFormat"));
        BranchInst::Create(returnBB, objcaseBB);
        
        // Block sw.default (doublecaseBB)
        llvm::Type *doublePtrType = llvm::Type::getDoublePtrTy(llvm::getGlobalContext());
        //doubleRef = (double*)jsValueRef;
        CastInst* doubleRef = llvm::CastInst::Create(llvm::Instruction::BitCast, jsValueRef, doublePtrType, "asDouble", doublecaseBB);
        //*doubleRef =value
        LoadInst* doubleValue = new llvm::LoadInst(doubleRef, "", false, doublecaseBB);
        doubleValue->setAlignment(8);
        snprintfParams.push_back(bufferValue);
        snprintfParams.push_back(lengthValue);
        snprintfParams.push_back(context.getVariableRef("doubleFormat"));
        snprintfParams.push_back(doubleValue);
        BranchInst::Create(returnBB, doublecaseBB);

        // Block return (truecaseBB)
    	llvm::Function* snprintfFunc = context.getFunction("snprintf");
        CallInst* int32_call14 = CallInst::Create(snprintfFunc, snprintfParams, "call_snprintf", doublecaseBB);
        llvm::ReturnInst::Create(llvm::getGlobalContext(), int32_call14, returnBB);

    	std::cout << "Creating function: " << name << std::endl;
	}
	
	std::vector<llvm::Value*> args;
	args.push_back(jsValue);
	args.push_back(buffer);
	args.push_back(bufferLength);
	CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
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

llvm::Value *CodeGenJSValue32FunctionGetHashCode(CodeGenContext& context, llvm::Value *jsValueRef)
{
    if (NULL == jsValueRef)
    {
        std::cout <<  __FUNCTION__ << " BADPARAM" << std::endl;
        return NULL;
    }

    llvm::Type *type = jsValueRef->getType();
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    if (type != jsValuePtrType)
    {
        llvm::Value *jsValueTempRef = CodeGenJSValue32Construct(context, "jsValueTempRef");
        CodeGenJSValue32FunctionCopy(context, jsValueTempRef, jsValueRef);
        jsValueRef = jsValueTempRef;
    }

	llvm::Function *function = context.getFunction("JSValueHash");
	if (function == NULL)
	{
        std::cout <<  __FUNCTION__ << " Can't find JSValueHash()" << std::endl;
		context.abort();
		return NULL;
	}
	
	std::vector<llvm::Value*> args;
	args.push_back(jsValueRef);
	CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
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

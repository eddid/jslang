/*
*  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
*  Copyright (C) 2001 Peter Kelly (pmk@post.com)
*  Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
*  Copyright (C) 2007 Cameron Zwarich (cwzwarich@uwaterloo.ca)
*  Copyright (C) 2007 Maks Orlovich
*  Copyright (C) 2007 Eric Seidel <eric@webkit.org>
*  Copyright (C) 2014 Eddid Zhang <zhangheng607@163.com>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
*
*/
#include "NodeInfo.h"
#include "CodeGen.h"
#include <iostream>
#include "llvm/Support/CFG.h"
#include <limits>

namespace JSC {

bool operator==(const Identifier& a, const Identifier& b)
{
    return 0 == a.name.compare(b.name);
}

bool operator!=(const Identifier& a, const Identifier& b)
{
    return 0 != a.name.compare(b.name);
}

bool operator==(const Identifier& a, const char* b)
{
    return 0 == a.name.compare(b);
}

bool operator!=(const Identifier& a, const char* b)
{
    return 0 != a.name.compare(b);
}

typedef struct
{
    ExpressionNode *expr;     //the expression
    llvm::BasicBlock *holdBB; //the block hold bitcode of the expression
    llvm::BasicBlock *trueBB; //the true branch of the expression
    llvm::BasicBlock *falseBB;//the false branch of the expression
}LogicalInfo;

/**
 * push all sub exprs of expr in a std::vector<LogicalInfo>, but only nonconstant leaf exprs holdBB isn't NULL
 * the first expr(the input expr) hold the end block(we get endBB = logicalInfos[0].holdBB)
 * the first nonconstant leaf expr(since it uses current block), other nonconstant leaf hold the block for itself
 * instead of do it nesting, vector.insert is a better way
 *
 * for exampe, (a # b) # ((b # c) # d):
 *          [#]
 *        /     \
 *      [#]      [#]
 *    /   \     /   \
 *   (a)  (b)  [#]  (e)
 *            /   \
 *           (c)  (d)
 * 
 * step 1. 
 *         (a # b) # ((c # d) # e)
 * step 2. 
 *         (a # b) # ((c # d) # e)
 *         (a # b)
 *         ((c # d) # e)
 * step 3. 
 *         (a # b) # ((c # d) # e)
 *         (a # b)
 *         a
 *         b
 *         ((c # d) # e)
 * step 4. 
 *         (a # b) # ((c # d) # e)
 *         (a # b)
 *         a
 *         b
 *         ((c # d) # e)
 *         (c # d)
 *         e
 * step 5. 
 *         (a # b) # ((c # d) # e)
 *         (a # b)
 *         a
 *         b
 *         ((c # d) # e)
 *         (c # d)
 *         c
 *         d
 *         e
 *
 * note: # means and, or, not
 *
 * the branch logical of LogicalInfo:
 * before do step1, the input expr should have trueBB and falseBB(no matter they are the same or not)
 * now, we are in the run loop:
 * case1, if expr is expr1 && expr2:
 *     if expr1 == false, goto expr.falseBB,
 *     if expr1 == true, goto expr2.holdBB,
 *     if expr2 == false, goto expr.falseBB,
 *     if expr2 == true, goto expr.trueBB,
 *  so, if expr1 is constant, expr2 inherits expr.holdBB
 *  or, expr1 inherits expr.holdBB, expr2 needs create a new holdBB
 * case2, if expr is expr1 || expr2:
 *     if expr1 == false, goto expr2.holdBB,
 *     if expr1 == true, goto expr.trueBB,
 *     if expr2 == false, goto expr.falseBB,
 *     if expr2 == true, goto expr.trueBB,
 *  so, if expr1 is constant, expr2 inherits expr.holdBB
 *  or, expr1 inherits expr.holdBB, expr2 needs create a new holdBB
 * case3, if expr is !expr1:
 *     if expr1 == false, goto expr.trueBB,
 *     if expr1 == true, goto expr.falseBB,
 *  so, expr1 inherits expr.holdBB, no need to create a new holdBB
 * default, expr is a complex expression, keep it as a leaf expr(the holdBB != NULL)
 *
 * note: expr1 and expr2 should not be constant at the same time, it means expr is a constant, the upper loop should deal with it
 *
 */
void initLogicalInfo(std::vector<LogicalInfo> &logicalInfos, ExpressionNode *expr, llvm::BasicBlock *trueBB, llvm::BasicBlock *falseBB)
{
    LogicalInfo logicalInfoExpr1;
    LogicalInfo logicalInfoExpr2;
    LogicalInfo *logicalInfoCurrent;
    LogicalOpNode *logicalOpNode;
    LogicalNotNode *logicalNotNode;
    llvm::BasicBlock *endBB = NULL;
    OpcodeID opcodeID;
    size_t index;
    double result;

    //need check isConstant() of expr ?
    //the caller should call isConstant() before call initLogicalInfo()
    if (expr->isConstant(result))
    {
        return ;
    }
    
    logicalInfoExpr1.expr = expr;
    if ((NULL != trueBB) && (NULL != falseBB))
    {
        logicalInfoExpr1.holdBB = NULL;
        logicalInfoExpr1.trueBB = trueBB;
        logicalInfoExpr1.falseBB = falseBB;
    }
    else
    {
        endBB = BasicBlock::Create(getGlobalContext(), "check.end");
        logicalInfoExpr1.holdBB = endBB;
        logicalInfoExpr1.trueBB = logicalInfoExpr1.holdBB;
        logicalInfoExpr1.falseBB = logicalInfoExpr1.holdBB;
    }
    logicalInfos.push_back(logicalInfoExpr1);
    for (index = 0; index < logicalInfos.size(); index++)
    {
        logicalInfoCurrent = &logicalInfos[index];
        opcodeID = logicalInfoCurrent->expr->opcodeID();
        if (op_and == opcodeID)
        {
            logicalOpNode = (LogicalOpNode *)logicalInfoCurrent->expr;

            //if one of subexpr is constant, don't create block for it
            if (logicalOpNode->expr1()->isConstant(result))
            {
                // br(1 && X) -> br(X).
                // br(0 && X) -> false.
                memcpy((void *)&logicalInfoExpr2, (const void *)logicalInfoCurrent, sizeof(LogicalInfo));
                logicalInfoExpr2.expr = logicalOpNode->expr2();

                logicalInfos[index].holdBB = NULL;
                logicalInfos.insert(logicalInfos.begin()+index+1, logicalInfoExpr2);
            }
            else if (logicalOpNode->expr2()->isConstant(result) && result)
            {
                // br(X && 1) -> br(X).
                // br(X && 0) -> false.  can't deal with
                memcpy((void *)&logicalInfoExpr1, (const void *)logicalInfoCurrent, sizeof(LogicalInfo));
                logicalInfoExpr1.expr = logicalOpNode->expr1();

                logicalInfos[index].holdBB = NULL;
                logicalInfos.insert(logicalInfos.begin()+index+1, logicalInfoExpr1);
            }
            else
            {
                memcpy((void *)&logicalInfoExpr2, (const void *)logicalInfoCurrent, sizeof(LogicalInfo));
                logicalInfoExpr2.expr = logicalOpNode->expr2();
                logicalInfoExpr2.holdBB = BasicBlock::Create(getGlobalContext(), "check.and");
    
                memcpy((void *)&logicalInfoExpr1, (const void *)logicalInfoCurrent, sizeof(LogicalInfo));
                logicalInfoExpr1.expr = logicalOpNode->expr1();
                logicalInfoExpr1.trueBB = logicalInfoExpr2.holdBB;

                logicalInfos[index].holdBB = NULL;
                logicalInfos.insert(logicalInfos.begin()+index+1, logicalInfoExpr2);
                logicalInfos.insert(logicalInfos.begin()+index+1, logicalInfoExpr1);
            }
        }
        else if (op_or == opcodeID)
        {
            logicalOpNode = (LogicalOpNode *)logicalInfoCurrent->expr;

            //if one of subexpr is constant, don't create block for it
            if (logicalOpNode->expr1()->isConstant(result))
            {
                // br(1 || X) -> true.
                // br(0 || X) -> br(X).
                memcpy((void *)&logicalInfoExpr2, (const void *)logicalInfoCurrent, sizeof(LogicalInfo));
                logicalInfoExpr2.expr = logicalOpNode->expr2();

                logicalInfos[index].holdBB = NULL;
                logicalInfos.insert(logicalInfos.begin()+index+1, logicalInfoExpr2);
            }
            else if (logicalOpNode->expr2()->isConstant(result) && !result)
            {
                // br(X || 0) -> br(X).
                // br(X || 1) -> true.  can't deal with
                memcpy((void *)&logicalInfoExpr1, (const void *)logicalInfoCurrent, sizeof(LogicalInfo));
                logicalInfoExpr1.expr = logicalOpNode->expr1();

                logicalInfos[index].holdBB = NULL;
                logicalInfos.insert(logicalInfos.begin()+index+1, logicalInfoExpr1);
            }
            else
            {
                memcpy((void *)&logicalInfoExpr2, (const void *)logicalInfoCurrent, sizeof(LogicalInfo));
                logicalInfoExpr2.expr = logicalOpNode->expr2();
                logicalInfoExpr2.holdBB = BasicBlock::Create(getGlobalContext(), "check.or");
    
                memcpy((void *)&logicalInfoExpr1, (const void *)logicalInfoCurrent, sizeof(LogicalInfo));
                logicalInfoExpr1.expr = logicalOpNode->expr1();
                logicalInfoExpr1.falseBB = logicalInfoExpr2.holdBB;

                logicalInfos[index].holdBB = NULL;
                logicalInfos.insert(logicalInfos.begin()+index+1, logicalInfoExpr2);
                logicalInfos.insert(logicalInfos.begin()+index+1, logicalInfoExpr1);
            }
        }
        else if (op_not == opcodeID)
        {
            logicalNotNode = (LogicalNotNode *)logicalInfoCurrent->expr;
            
            memcpy((void *)&logicalInfoExpr1, (const void *)logicalInfoCurrent, sizeof(LogicalInfo));
            logicalInfoExpr1.expr = logicalNotNode->expr();
            logicalInfoExpr1.falseBB = logicalInfoCurrent->trueBB;
            logicalInfoExpr1.trueBB = logicalInfoCurrent->falseBB;

            logicalInfos[index].holdBB = NULL;
            logicalInfos.insert(logicalInfos.begin()+index+1, logicalInfoExpr1);
        }
        else
        {
            // push as complex expression, do nothing
        }
    }

    // set holdBB of first leaf expr to NULL, and set holdBB of the top expr(input expr) to endBB
    if (NULL != endBB)
    {
        for (index = 0; index < logicalInfos.size(); index++)
        {
            if (endBB == logicalInfos[index].holdBB)
            {
                logicalInfos[index].holdBB = NULL;
                break;
            }
        }
        logicalInfos[0].holdBB = endBB;
    }
}

LogicalInfo *findLogicalInfo(std::vector<LogicalInfo> &logicalInfos, ExpressionNode *expr)
{
    std::vector<LogicalInfo>::const_iterator it;
    
    for (it = logicalInfos.begin(); it != logicalInfos.end(); it++)
    {
        if (expr == (*it).expr)
        {
            return (LogicalInfo *)&(*it);
        }
    }

    return NULL;
}

static llvm::Value *EmitLogicalOptionExpr(CodeGenContext& context, std::vector<LogicalInfo> &logicalInfos, ExpressionNode *expr, PHINode *phiNode)
{
    llvm::BasicBlock *endBB = logicalInfos[0].holdBB;
    LogicalInfo *logicalInfo = NULL;
    llvm::Value *condV = NULL;
    OpcodeID opcodeID = expr->opcodeID();
    double constantValue;

    // Handle X && Y in a condition.
    if (op_and == opcodeID)
    {
        LogicalOpNode *logicalExpr = (LogicalOpNode *)expr;
        // If we have "1 && X", simplify the code.  "0 && X" would have constant
        // folded if the case was simple enough.

        if ((NULL == logicalExpr->expr1()) || (NULL == logicalExpr->expr2()))
        {
            std::cout <<  __FUNCTION__ << " BAD LogicalOpNode: expr1=" << logicalExpr->expr1() << ",expr2=" << logicalExpr->expr2() << std::endl;
            context.abort();
            return NULL;
        }

        if (logicalExpr->expr1()->isConstant(constantValue))
        {
            if (constantValue)
            {
                // br(1 && X) -> br(X).
                return EmitLogicalOptionExpr(context, logicalInfos, logicalExpr->expr2(), phiNode);
            }
            else
            {
                //since expr1 is constant, it isn't in logicalInfos, use expr2 to get the info
                logicalInfo = findLogicalInfo(logicalInfos, logicalExpr->expr2());
                if (NULL == logicalInfo)
                {
                    std::cout <<  __FUNCTION__ << " Can't find: expr=" << expr << " in logicalInfos" << std::endl;
                    context.abort();
                    return NULL;
                }
                // br(0 && X) -> false.
                condV = llvm::ConstantInt::getFalse(llvm::getGlobalContext());
                if (logicalInfo->trueBB != logicalInfo->falseBB)
                {
                    BranchInst::Create(logicalInfo->falseBB, context.currentBlock());
                    if (logicalInfo->falseBB == endBB)
                    {
                        phiNode->addIncoming(condV, context.currentBlock());
                    }
                }
                return condV;
            }
        }

        // If we have "X && 1", simplify the code to use an uncond branch.
        // "X && 0" would have been constant folded to 0.
        // can't deal with "X && 0" right now
        if (logicalExpr->expr2()->isConstant(constantValue) && constantValue)
        {
            // need execute expr1 no matter expr2 is 1 or 0, since it is may a complex expression
            condV = EmitLogicalOptionExpr(context, logicalInfos, logicalExpr->expr1(), phiNode);
            if (!constantValue)
            {
                //since expr2 is constant, it isn't in logicalInfos, use expr1 to get the info
                logicalInfo = findLogicalInfo(logicalInfos, logicalExpr->expr1());
                if (NULL == logicalInfo)
                {
                    std::cout <<  __FUNCTION__ << " Can't find: expr=" << expr << " in logicalInfos" << std::endl;
                    context.abort();
                    return NULL;
                }
                // br(X && 0) -> false.
                condV = llvm::ConstantInt::getFalse(llvm::getGlobalContext());
                if (logicalInfo->trueBB != logicalInfo->falseBB)
                {
                    BranchInst::Create(logicalInfo->falseBB, context.currentBlock());
                    if (logicalInfo->falseBB == endBB)
                    {
                        phiNode->addIncoming(condV, context.currentBlock());
                    }
                }
            }

            // br(X && 1) -> br(X).
            return condV;
        }
  
        // Emit the LHS as a conditional.  If the LHS conditional is false, we
        // want to jump to the FalseBlock.
        condV = EmitLogicalOptionExpr(context, logicalInfos, logicalExpr->expr1(), phiNode);

        // Any temporaries created here are conditional.
        condV = EmitLogicalOptionExpr(context, logicalInfos, logicalExpr->expr2(), phiNode);
        
        return condV;
    }

    if (op_or == opcodeID)
    {
        LogicalOpNode *logicalExpr = (LogicalOpNode *)expr;
        // If we have "0 || X", simplify the code.  "1 || X" would have constant
        // folded if the case was simple enough.
        if ((NULL == logicalExpr->expr1()) || (NULL == logicalExpr->expr2()))
        {
            std::cout <<  __FUNCTION__ << " BAD LogicalOpNode: expr1=" << logicalExpr->expr1() << ",expr2=" << logicalExpr->expr2() << std::endl;
            context.abort();
            return NULL;
        }

        if (logicalExpr->expr1()->isConstant(constantValue))
        {
            if (constantValue)
            {
                //since expr1 is constant, it isn't in logicalInfos, use expr2 to get the info
                logicalInfo = findLogicalInfo(logicalInfos, logicalExpr->expr2());
                if (NULL == logicalInfo)
                {
                    std::cout <<  __FUNCTION__ << " Can't find: expr=" << expr << " in logicalInfos" << std::endl;
                    context.abort();
                    return NULL;
                }
                // br(1 || X) -> true.
                condV = llvm::ConstantInt::getTrue(llvm::getGlobalContext());
                if (logicalInfo->trueBB != logicalInfo->falseBB)
                {
                    BranchInst::Create(logicalInfo->trueBB, context.currentBlock());
                    if (logicalInfo->trueBB == endBB)
                    {
                        phiNode->addIncoming(condV, context.currentBlock());
                    }
                }
                return condV;
            }
            else
            {
                // br(0 || X) -> br(X).
                return EmitLogicalOptionExpr(context, logicalInfos, logicalExpr->expr2(), phiNode);
            }
        }
  
        // If we have "X || 0", simplify the code to use an uncond branch.
        // "X || 1" would have been constant folded to 1.
        // can't deal with "X || 1"
        if (logicalExpr->expr2()->isConstant(constantValue) && !constantValue)
        {
            // need execute expr1 no matter expr2 is 1 or 0, since it is may a complex expression
            condV = EmitLogicalOptionExpr(context, logicalInfos, logicalExpr->expr1(), phiNode);
            if (constantValue)
            {
                //since expr2 is constant, it isn't in logicalInfos, use expr1 to get the info
                logicalInfo = findLogicalInfo(logicalInfos, logicalExpr->expr1());
                if (NULL == logicalInfo)
                {
                    std::cout <<  __FUNCTION__ << " Can't find: expr=" << expr << " in logicalInfos" << std::endl;
                    context.abort();
                    return NULL;
                }
                // br(X || 1) -> true.
                condV = llvm::ConstantInt::getTrue(llvm::getGlobalContext());
                if (logicalInfo->trueBB != logicalInfo->falseBB)
                {
                    BranchInst::Create(logicalInfo->trueBB, context.currentBlock());
                    if (logicalInfo->trueBB == endBB)
                    {
                        phiNode->addIncoming(condV, context.currentBlock());
                    }
                }
            }

            // br(X || 0) -> br(X).
            return condV;
        }
  
        // Emit the LHS as a conditional.  If the LHS conditional is true, we
        // want to jump to the TrueBlock.

        condV = EmitLogicalOptionExpr(context, logicalInfos, logicalExpr->expr1(), phiNode);
  
        // Any temporaries created here are conditional.
        condV = EmitLogicalOptionExpr(context, logicalInfos, logicalExpr->expr2(), phiNode);
        
        return condV;
    }

    // br(!x, t, f) -> br(x, f, t)
    if (op_not == opcodeID)
    {
        LogicalNotNode *notExpr = (LogicalNotNode *)expr;
        
        logicalInfo = findLogicalInfo(logicalInfos, expr);
        if (NULL == logicalInfo)
        {
            std::cout <<  __FUNCTION__ << " Can't find: expr=" << expr << " in logicalInfos" << std::endl;
            context.abort();
            return NULL;
        }

        condV = EmitLogicalOptionExpr(context, logicalInfos, notExpr->expr(), phiNode);
        if (logicalInfo->trueBB == logicalInfo->falseBB)
        {
            // expr flow to check.end, so we need to do "!expr" by ourself
            llvm::Value *trueV = llvm::ConstantInt::getTrue(llvm::getGlobalContext());
            condV = BinaryOperator::Create(Instruction::Xor, condV, trueV, "lnot", context.currentBlock());
        }

        return condV;
    }

#if 0
  if (const ConditionalOperator *CondOp = dyn_cast<ConditionalOperator>(expr)) {
    // br(c ? x : y, t, f) -> br(c, br(x, t, f), br(y, t, f))
    llvm::BasicBlock *LHSBlock = createBasicBlock("cond.true");
    llvm::BasicBlock *RHSBlock = createBasicBlock("cond.false");

    ConditionalEvaluation cond(*this);
    EmitLogicalOptionExpr(CondOp->getCond(), LHSBlock, RHSBlock);

    cond.begin(*this);
    EmitBlock(LHSBlock);
    EmitLogicalOptionExpr(CondOp->expr1(), TrueBlock, FalseBlock);
    cond.end(*this);

    cond.begin(*this);
    EmitBlock(RHSBlock);
    EmitLogicalOptionExpr(CondOp->expr2(), TrueBlock, FalseBlock);
    cond.end(*this);

    return;
  }

  if (const CXXThrowExpr *Throw = dyn_cast<CXXThrowExpr>(expr)) {
    // Conditional operator handling can give us a throw expression as a
    // condition for a case like:
    //   br(c ? throw x : y, t, f) -> br(c, br(throw x, t, f), br(y, t, f)
    // Fold this to:
    //   br(c, throw x, br(y, t, f))
    EmitCXXThrowExpr(Throw, /*KeepInsertionPoint*/false);
    return;
  }
#endif

    logicalInfo = findLogicalInfo(logicalInfos, expr);
    if (NULL == logicalInfo)
    {
        std::cout <<  __FUNCTION__ << " Can't find: expr=" << expr << " in logicalInfos" << std::endl;
        context.abort();
        return NULL;
    }

	if ((NULL != logicalInfo->holdBB) && (endBB != logicalInfo->holdBB))
	{
		// the first check use current block, the holdBB is NULL or endBB
        context.pushBlock(logicalInfo->holdBB);
	}

    if (expr->isConstant(constantValue))
    {
        if (constantValue)
        {
            // br(1) -> true.
            condV = llvm::ConstantInt::getTrue(llvm::getGlobalContext());
#if 1
            if (logicalInfo->trueBB != logicalInfo->falseBB)
            {
                BranchInst::Create(logicalInfo->trueBB, context.currentBlock());
                if (logicalInfo->trueBB == endBB)
                {
                    phiNode->addIncoming(condV, context.currentBlock());
                }
            }
#endif
        }
        else
        {
            // br(0) -> false.
            condV = llvm::ConstantInt::getFalse(llvm::getGlobalContext());
#if 1
            if (logicalInfo->trueBB != logicalInfo->falseBB)
            {
                BranchInst::Create(logicalInfo->falseBB, context.currentBlock());
                if (logicalInfo->falseBB == endBB)
                {
                    phiNode->addIncoming(condV, context.currentBlock());
                }
            }
#endif
        }
    }
    else
    {
        // Emit the code with the fully general case.
        condV = CodeGenJSValue32FunctionIsTrue(context, expr->emitBytecode(context, NULL));
        if (logicalInfo->trueBB != logicalInfo->falseBB)
        {
            BranchInst::Create(logicalInfo->trueBB, logicalInfo->falseBB, condV, context.currentBlock());
            if ((logicalInfo->trueBB == endBB) || (logicalInfo->falseBB == endBB))
            {
                phiNode->addIncoming(condV, context.currentBlock());
            }
        }
    }

    return condV;
}

bool Node::isConstant(double &result) const
{
    switch (m_opcodeID)
    {
    case op_const_undefined:
        result = std::numeric_limits<double>::quiet_NaN();
        return true;
        break;

    case op_const_null:
        result = 0.0;
        return true;
        break;

    case op_const_boolean:
        {
            BooleanNode *booleanExpr = (BooleanNode *)this;
            result = booleanExpr->value() ? 1.0 : 0.0;
        }
        return true;
        break;

    case op_const_number:
        {
            NumberNode *numberExpr = (NumberNode *)this;
            result = numberExpr->value();
        }
        return true;
        break;

    case op_const_string:
        {
            StringNode *stringExpr = (StringNode *)this;
            char *endptr;
            std::string value = stringExpr->value().name;
            result = (0 < value.length());
            result = strtod(value.c_str(), &endptr);
            if (*endptr != '\0')
            {
                result = std::numeric_limits<double>::quiet_NaN();
            }
        }
        return true;
        break;

    case op_negate:
    case op_bitnot:
    case op_not:
        {
            UnaryOpNode *unaryOpNode = (UnaryOpNode *)this;
            ExpressionNode *expr = unaryOpNode->expr();
            if (expr->isConstant(result))
            {
                if (op_negate == m_opcodeID)
                {
                    result = -result;
                }
                else if (op_bitnot == m_opcodeID)
                {
                    int iResult = (int)result;
                    iResult = ~iResult;
                    result = (double)iResult;
                }
                else
                {
                    result = result ? 1.0 : 0.0;
                }
                return true;
            }
            return false;
        }
        break;

    case op_add:
    case op_sub:
    case op_mul:
    case op_div:
    case op_mod:
    case op_lshift:
    case op_rshift:
    case op_urshift:
    case op_bitand:
    case op_bitxor:
    case op_bitor:
    case op_eq:
    case op_neq:
    case op_less:
    case op_lesseq:
    case op_greater:
    case op_greatereq:
    case op_and:
    case op_or:
        {
            BinaryOpNode *binaryOpNode = (BinaryOpNode *)this;
            ExpressionNode *expr1 = binaryOpNode->expr1();
            ExpressionNode *expr2 = binaryOpNode->expr2();
            double rhRsult;
            if (!(expr1->isConstant(result)) || !(expr2->isConstant(rhRsult)))
            {
                return false;
            }
            switch (m_opcodeID)
            {
                case op_add:
                    result = result + rhRsult;
                    break;

                case op_sub:
                    result = result - rhRsult;
                    break;

                case op_mul:
                    result = result * rhRsult;
                    break;

                case op_div:
                    result = result / rhRsult;
                    break;

                case op_mod:
                    result = (double)((int)result % (int)rhRsult);
                    break;

                case op_lshift:
                    result = result + rhRsult;
                    break;

                case op_rshift:
                    result = result + rhRsult;
                    break;

                case op_urshift:
                    result = result + rhRsult;
                    break;

                case op_bitand:
                    result = (double)((int)result & (int)rhRsult);
                    break;

                case op_bitxor:
                    result = (double)((int)result ^ (int)rhRsult);
                    break;

                case op_bitor:
                    result = (double)((int)result | (int)rhRsult);
                    break;

                case op_eq:
                    result = (result == rhRsult) ? 1.0 : 0.0;
                    break;

                case op_neq:
                    result = (result != rhRsult) ? 1.0 : 0.0;
                    break;

                case op_less:
                    result = (result < rhRsult) ? 1.0 : 0.0;
                    break;

                case op_lesseq:
                    result = (result <= rhRsult) ? 1.0 : 0.0;
                    break;

                case op_greater:
                    result = (result > rhRsult) ? 1.0 : 0.0;
                    break;

                case op_greatereq:
                    result = (result >= rhRsult) ? 1.0 : 0.0;
                    break;

                case op_and:
                    result = (result && rhRsult) ? 1.0 : 0.0;
                    break;

                case op_or:
                    result = (result || rhRsult) ? 1.0 : 0.0;
                    break;

				default:
					return false;
					break;
            }
            return true;
        }
        break;

    default:
        break;
    }

    return false;
}

/*
    Details of the emitBytecode function.

    Return value: The register holding the production's value.
             dst: An optional parameter specifying the most efficient destination at
                  which to store the production's value. The callee must honor dst.

    The dst argument provides for a crude form of copy propagation. For example,

        x = 1

    becomes
    
        load r[x], 1
    
    instead of 

        load r0, 1
        mov r[x], r0
    
    because the assignment node, "x =", passes r[x] as dst to the number node, "1".
*/

// ------------------------------ JavaScriptCore/ThrowableExpressionData --------------------------------

RegisterID* ThrowableExpressionData::emitThrowError(CodeGenContext& context, ErrorType type, const char* message)
{
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

inline RegisterID* ThrowableExpressionData::emitThrowError(CodeGenContext& context, ErrorType type, const char* messageTemplate, const Identifier& label)
{
    return emitThrowError(context, type, messageTemplate, label.name.c_str());
}

// ------------------------------ JavaScriptCore/StatementNode --------------------------------

void StatementNode::setLoc(int firstLine, int lastLine)
{
    m_line = firstLine;
    m_lastLine = lastLine;
}

// ------------------------------ JavaScriptCore/SourceElements --------------------------------

void SourceElements::append(StatementNode* statement)
{
    if (statement->isEmptyStatement())
        return;
    m_statements.push_back(statement);
}

inline StatementNode* SourceElements::singleStatement() const
{
    size_t size = m_statements.size();
    return size == 1 ? m_statements[0] : 0;
}

inline StatementNode* SourceElements::lastStatement() const
{
    size_t size = m_statements.size();
    return size ? m_statements[size - 1] : 0;
}

// ------------------------------ JavaScriptCore/NullNode -------------------------------------

RegisterID* NullNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    return CodeGenJSValue32ConstructForNULL(context);
}

// ------------------------------ JavaScriptCore/BooleanNode ----------------------------------

RegisterID *BooleanNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    return CodeGenJSValue32ConstructForBool(context, m_value);
}

// ------------------------------ JavaScriptCore/NumberNode -----------------------------------

RegisterID* NumberNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), m_value);
}

// ------------------------------ JavaScriptCore/StringNode -----------------------------------

RegisterID* StringNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    const std::string &name = value().name;
    return CodeGenJSObjectMakeStringCall(context, name.c_str(), name.length());
}

// ------------------------------ JavaScriptCore/RegExpNode -----------------------------------

RegisterID* RegExpNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
	llvm::Function *function = context.getFunction("_ejs_regexp_new_utf8");
    if (NULL == function)
    {
        std::cout <<  __FUNCTION__ << " Can't find _ejs_regexp_new_utf8()" << std::endl;
        context.abort();
        return NULL;
    }

    llvm::Constant *patternRef;
    llvm::Constant *flagsRef;

    llvm::Type *int8Type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
    llvm::Type *int8PtrType = llvm::PointerType::get(int8Type, 0);
    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Constant *zero = llvm::Constant::getNullValue(int32Type);
    std::vector<llvm::Constant*> indices;

    indices.push_back(zero);
    indices.push_back(zero);

    if (m_pattern.name.length() <= 0)
    {
        patternRef = llvm::Constant::getNullValue(int8PtrType);
    }
    else
    {
        llvm::Constant *patternArray = llvm::ConstantDataArray::getString(llvm::getGlobalContext(), m_pattern.name);
        llvm::ArrayType* patternType = llvm::ArrayType::get(int8Type, m_pattern.name.length() + 1);
        llvm::GlobalVariable *patternGlobalVariable = new llvm::GlobalVariable(*context.getModule(), patternType, true, llvm::GlobalValue::PrivateLinkage, patternArray, m_pattern.name);
        patternRef = llvm::ConstantExpr::getGetElementPtr(patternGlobalVariable, indices);
    }

    if (m_flags.name.length() <= 0)
    {
        flagsRef = llvm::Constant::getNullValue(int8PtrType);
    }
    else
    {
        llvm::Constant *flagsArray = llvm::ConstantDataArray::getString(llvm::getGlobalContext(), m_flags.name);
        llvm::ArrayType* flagsType = llvm::ArrayType::get(int8Type, m_flags.name.length() + 1);
        llvm::GlobalVariable *flagsGlobalVariable = new llvm::GlobalVariable(*context.getModule(), flagsType, true, llvm::GlobalValue::PrivateLinkage, flagsArray, m_flags.name);
        flagsRef = llvm::ConstantExpr::getGetElementPtr(flagsGlobalVariable, indices);
    }

	std::vector<RegisterID*> args;
	args.push_back(patternRef);
	args.push_back(flagsRef);
	llvm::CallInst *call = llvm::CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
	{
        call->setCallingConv(CallingConv::C);
        call->setTailCall(false);
	}

	return call;
}

// ------------------------------ JavaScriptCore/ThisNode -------------------------------------

RegisterID* ThisNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    RegisterID *thisRef = context.getVariableRef("this");
    if (NULL == thisRef)
    {
        std::cout <<  __FUNCTION__ << " Can't find this" << std::endl;
        context.abort();
        return NULL;
    }
    
    return thisRef;
}

// ------------------------------ JavaScriptCore/ResolveNode ----------------------------------

bool ResolveNode::isPure(CodeGenContext& context) const
{
    return true;
}

RegisterID* ResolveNode::createVariable(CodeGenContext &context, bool inLocal)
{
    llvm::Function *currentFunction = context.currentFunction();
    llvm::Function *mainFunction = context.mainFunction();
    RegisterID *idRef = NULL;
    
    // if the variable is create in main, create as global
    if (!inLocal || (currentFunction == mainFunction))
    {
        llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
        idRef = new GlobalVariable(*context.getModule(), jsValueType, false, GlobalValue::ExternalLinkage, ConstantAggregateZero::get(jsValueType), m_ident.name);
        context.setVariableRef(m_ident.name, idRef, false);
    }
    else
    {
        idRef = CodeGenJSValue32Construct(context, m_ident.name.c_str());
        context.setVariableRef(m_ident.name, idRef, true);
    }

    return idRef;
}

RegisterID* ResolveNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (m_ident.name.length() <= 0)
    {
        std::cout <<  __FUNCTION__ << " m_ident=NULL" << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *idRef = context.getVariableRef(m_ident.name);
    if (NULL == idRef)
    {
        //find this
        RegisterID *thisRef = context.getVariableRef("this");
        //find this.m_ident
        RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, thisRef, m_ident.name);

        idRef = CodeGenJSValue32Construct(context, m_ident.name.c_str());
        CodeGenJSValue32FunctionCopy(context, idRef, resultValue);
        context.setVariableRef(m_ident.name, idRef, true);
    }

    return idRef;//new LoadInst(idRef, "", false, context.currentBlock());
}

// ------------------------------ JavaScriptCore/ArrayNode ------------------------------------

RegisterID* ArrayNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    Function *function = context.getFunction("_ejs_array_new");
    if (NULL == function)
    {
        std::cout <<  __FUNCTION__ << " Can't find _ejs_array_new()" << std::endl;
        context.abort();
        return NULL;
    }

    //get total count
    int elementCount = m_elision;
    bool fillOrNot = (0 != m_elision);
    ElementNode* elementNodeIndex;
    for (elementNodeIndex = m_element; (NULL != elementNodeIndex); elementNodeIndex = elementNodeIndex->next())
    {
        if (0 != elementNodeIndex->elision())
        {
            elementCount += elementNodeIndex->elision();
            fillOrNot = 1;
        }
        if (NULL != elementNodeIndex->value())
        {
            elementCount++;
        }
    }

    //make array
    llvm::IntegerType *int1Type = llvm::Type::getInt1Ty(llvm::getGlobalContext());
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
	std::vector<RegisterID*> args;
	args.push_back(llvm::ConstantInt::get(int32Type, elementCount));
	args.push_back(llvm::ConstantInt::get(int1Type, fillOrNot));
	llvm::CallInst *arrayRef = llvm::CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    {
        arrayRef->setCallingConv(CallingConv::C);
        arrayRef->setTailCall(false);

        AttributeSet call_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;
        
        AttrBuilder B;
        B.addAttribute(Attribute::ZExt);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 0U, B);
        Attrs.push_back(PAS);
        
        call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        arrayRef->setAttributes(call_PAL);
    }
    //fill array
    int elementIndex = 0;
    llvm::Value *elementRef;
    for (elementNodeIndex = m_element; (NULL != elementNodeIndex); elementNodeIndex = elementNodeIndex->next())
    {
        //filled items have been inited when create
        elementIndex += elementNodeIndex->elision();
        if (NULL == elementNodeIndex->value())
        {
            continue;
        }
        elementRef = elementNodeIndex->value()->emitBytecode(context, dst);
        CodeGenJSObjectSetPropertyCall(context, arrayRef, llvm::ConstantInt::get(int32Type, elementIndex), elementRef);
        elementIndex++;
    }

    return arrayRef;
}

bool ArrayNode::isSimpleArray() const
{
    if (m_elision || m_optional)
        return false;
    for (ElementNode* ptr = m_element; ptr; ptr = ptr->next()) {
        if (ptr->elision())
            return false;
    }
    return true;
}

ArgumentListNode* ArrayNode::toArgumentList(CodeGenContext* globalData) const
{
    ASSERT(!m_elision && !m_optional);
    ElementNode* ptr = m_element;
    if (!ptr)
        return 0;
    ArgumentListNode* head = new ArgumentListNode(globalData, ptr->value());
    ArgumentListNode* tail = head;
    ptr = ptr->next();
    for (; ptr; ptr = ptr->next()) {
        ASSERT(!ptr->elision());
        tail = new ArgumentListNode(globalData, ptr->value(), tail);
    }
    return head;
}

// ------------------------------ JavaScriptCore/ObjectLiteralNode ----------------------------

RegisterID* ObjectLiteralNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    //call Object.create ( O [, Properties] ) or call _ejs_Object_create directly...
    Function *function = context.getFunction("_ejs_Object_create");
    if (NULL == function)
    {
        std::cout <<  __FUNCTION__ << " Can't find _ejs_Object_create()" << std::endl;
        context.abort();
        return NULL;
    }

    //create a object to hold property list
    llvm::Value *jsNullRef = context.getVariableRef("_ejs_null");
    std::vector<llvm::Value *> args;
    args.push_back(jsNullRef);
    llvm::Value *objectRef = CodeGenJSObjectFunctionCall(context, function, args);
	objectRef = CodeGenJSValue32ConstructIfNeeded(context, objectRef);

    if (NULL != m_list)
    {
        m_list->emitBytecode(context, objectRef);
    }

    return objectRef;
}

// ------------------------------ JavaScriptCore/PropertyListNode -----------------------------

RegisterID* PropertyListNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == dst) || (NULL == m_node) || (NULL == m_node->m_assign))
    {
        std::cout <<  __FUNCTION__ << " dst=" << dst << ",m_node=" << m_node << ",m_assign=" << m_node->m_assign << std::endl;
        context.abort();
        return NULL;
    }
    Function *constantFunction = context.getFunction("_ejs_object_define_value_property");
    if (NULL == constantFunction)
    {
        std::cout <<  __FUNCTION__ << " Can't find _ejs_object_define_value_property()" << std::endl;
        context.abort();
        return NULL;
    }
    Function *getterFunction = context.getFunction("_ejs_object_define_getter_property");
    if (NULL == getterFunction)
    {
        std::cout <<  __FUNCTION__ << " Can't find _ejs_object_define_getter_property()" << std::endl;
        context.abort();
        return NULL;
    }
    Function *setterFunction = context.getFunction("_ejs_object_define_setter_property");
    if (NULL == setterFunction)
    {
        std::cout <<  __FUNCTION__ << " Can't find _ejs_object_define_setter_property()" << std::endl;
        context.abort();
        return NULL;
    }

    llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    Function *function;
    RegisterID* key;
    RegisterID* value;
    RegisterID* flag = llvm::ConstantInt::get(int32Type, EJS_PROP_FLAGS_ENUMERABLE | EJS_PROP_FLAGS_ENUMERABLE_SET);
	std::vector<RegisterID*> args;
    for (PropertyListNode *it = this; it != NULL; it = it->next())
    {
        if ((NULL == it->m_node) || (NULL == it->m_node->m_assign))
        {
            std::cout <<  __FUNCTION__ << "node=" << it->m_node << ",assign=" << it->m_node->m_assign << std::endl;
            continue;
        }
        switch (it->m_node->m_type)
        {
            case PropertyNode::Constant:
                function = constantFunction;
                break;
    
            case PropertyNode::Getter:
                function = getterFunction;
                break;
    
            case PropertyNode::Setter:
                function = setterFunction;
                break;

            default:
                std::cout <<  __FUNCTION__ << " Unknown type=" << it->m_node->m_type << std::endl;
                function = NULL;
                break;
        }

        if (NULL == function)
        {
            continue;
        }
        value = it->m_node->m_assign->emitBytecode(context, dst);
        value = CodeGenJSValue32ConstructIfNeeded(context, value);
        const std::string &ident = it->m_node->identifier().name;
        if (ident.length() <= 0)
        {
            key = CodeGenJSValue32ConstructForDouble(context, it->m_node->number(), "");
        }
        else
        {
            key = CodeGenJSObjectMakeStringCall(context, ident.c_str(), ident.length());
			key = CodeGenJSValue32ConstructIfNeeded(context, key);
        }
        args.clear();
    	args.push_back(dst);
    	args.push_back(key);
    	args.push_back(value);
    	args.push_back(flag);
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
            
            call_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
            call->setAttributes(call_PAL);
        }
    }

    return NULL;
}

// ------------------------------ JavaScriptCore/BracketAccessorNode --------------------------------

RegisterID* BracketAccessorNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == m_base) || (NULL == m_subscript))
    {
        std::cout <<  __FUNCTION__ << " m_base=" << m_base << ",m_subscript=" << m_subscript << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);
    RegisterID *iteratorRef = m_subscript->emitBytecode(context, dst);

    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, iteratorRef);
    
    return resultValue;
}

// ------------------------------ JavaScriptCore/DotAccessorNode --------------------------------

RegisterID* DotAccessorNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_base)
    {
        std::cout <<  __FUNCTION__ << " BAD AssignDotNode" << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);

    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, m_ident.name);
    
    return resultValue;
}

// ------------------------------ JavaScriptCore/ArgumentListNode -----------------------------

RegisterID* ArgumentListNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_expr)
    {
        std::cout <<  __FUNCTION__ << " m_expr=NULL" << std::endl;
        context.abort();
        return NULL;
    }

    return m_expr->emitBytecode(context, dst);
}

// ------------------------------ JavaScriptCore/NewExprNode ----------------------------------

RegisterID* NewExprNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_expr)
    {
        std::cout <<  __FUNCTION__ << " m_expr=NULL" << std::endl;
        context.abort();
        return NULL;
    }

    llvm::Value * newThisRef = CodeGenJSValue32ConstructForUndefined(context, "new.this");

    llvm::Value *exprRef = m_expr->emitBytecode(context, dst);
    //NOTE: shoule check whether expr is just a function here, if yes, create a global varialbe as TYPE for it:
    if (llvm::isa<llvm::Function>(exprRef))
    {
        llvm::Function *exprFunction = (llvm::Function *)exprRef;
        exprRef = CodeGenJSObjectMakeFunctionCall(context, exprFunction, exprFunction->getName());
    }

    llvm::Value *int64Result;
    //get expr.prototype
    llvm::Value *prototypeRef = CodeGenJSValue32ConstructForUndefined(context, "prototype");
    int64Result = CodeGenJSObjectGetPropertyCall(context, exprRef, context.propertyNames->prototype.name);
    CodeGenJSValue32FunctionCopy(context, prototypeRef, int64Result);
    //get expr.create
    llvm::Value *createRef = CodeGenJSValue32ConstructForUndefined(context, "create");
    int64Result = CodeGenJSObjectGetPropertyCall(context, exprRef, context.getVariableRef("_ejs_Symbol_create"));
    CodeGenJSValue32FunctionCopy(context, createRef, int64Result);
    //check whether expr.create is valid
    llvm::Value *condV = CodeGenJSValue32FunctionIsUndefined(context, createRef);

    //create two BB to deal with two cases
    Function *currentFunction = context.currentFunction();
    BasicBlock *trueBB = BasicBlock::Create(getGlobalContext(), "new.runtime.function", currentFunction);
    BasicBlock *falseBB = BasicBlock::Create(getGlobalContext(), "new.buildin.type", currentFunction);
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "new.end", currentFunction);
    BranchInst::Create(trueBB, falseBB, condV, context.currentBlock());
    
    std::vector<llvm::Value*> args;
    //call _ejs_Object_create to create a default "Object" object
	Function *function = context.getFunction("_ejs_Object_create");
    context.pushBlock(trueBB);
    args.push_back(prototypeRef);
    llvm::Value *falseBBThisRef = CodeGenJSObjectFunctionCall(context, function, args);
    CodeGenJSValue32FunctionCopy(context, newThisRef, falseBBThisRef);
    BranchInst::Create(endBB, context.currentBlock());
    
    //call exprRef.create to create a object
    args.clear();
    context.pushBlock(falseBB);
    args.push_back(prototypeRef);
    llvm::Value *trueBBThisRef = CodeGenJSObjectConstructCall(context, createRef, exprRef, args);
    CodeGenJSValue32FunctionCopy(context, newThisRef, trueBBThisRef);
    BranchInst::Create(endBB, context.currentBlock());

    //do constructor of type
    args.clear();
    context.pushBlock(endBB);
    for (ArgumentListNode *it = m_args->m_listNode; it != NULL; it = it->next())
    {
        args.push_back(it->emitBytecode(context, dst));
    }
    llvm::Value * resultRef = CodeGenJSObjectConstructCall(context, exprRef, newThisRef, args);

    // value actually
    return newThisRef;
}

// ------------------------------ JavaScriptCore/EvalFunctionCallNode ----------------------------------

RegisterID* EvalFunctionCallNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    Function *function = context.getFunction("_ejs_eval");
    if (function == NULL)
    {
        std::cout <<  __FUNCTION__ << " Can't find _ejs_eval()" << std::endl;
        context.abort();
        return NULL;
    }
    
    RegisterID *param;
    ArgumentListNode* it;
    std::vector<llvm::Value *> args;
    for (it = m_args->m_listNode; it != NULL; it = it->next())
    {
        param = it->emitBytecode(context, dst);
        args.push_back(param);
    }

    llvm::Value * resultRef = CodeGenJSObjectFunctionCall(context, function, args);

    std::cout <<  __FUNCTION__ << "Creating method call: _ejs_eval" << std::endl;

    return resultRef;
}

/* Returns an LLVM type based on the identifier */
static Type *typeOf(CodeGenContext& context, const ExpressionNode* expr) 
{
    ResultType type = expr->resultDescriptor();
    if (expr->isNumber())
    {
        return Type::getDoubleTy(getGlobalContext());
    }
    else if (expr->isString())
    {
    }
    else if (expr->isNull())
    {
        return Type::getInt32Ty(getGlobalContext());
    }
    else if (expr->isPure(context))
    {
        return Type::getInt32Ty(getGlobalContext());
    }
    return Type::getVoidTy(getGlobalContext());
}

// ------------------------------ JavaScriptCore/FunctionCallValueNode ----------------------------------

RegisterID* FunctionCallValueNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_expr)
    {
        std::cout <<  __FUNCTION__ << " BAD FunctionCallDotNode" << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *objectRef = m_expr->emitBytecode(context, dst);

    //check objectRef type, it should be (union.ejsval *), but it may be int64_t since ABI can't return struct types
    objectRef = CodeGenJSValue32ConstructIfNeeded(context, objectRef);

    //do it like FunctionCallResolveNode 
    RegisterID *param;
    ArgumentListNode* it;
    std::vector<RegisterID*> args;
    for (it = m_args->m_listNode; it != NULL; it = it->next())
    {
        param = it->emitBytecode(context, dst);
        args.push_back(param);
    }
    llvm::Value * newThisRef = objectRef;
    llvm::Value * resultRef = CodeGenJSObjectConstructCall(context, objectRef, newThisRef, args);
    
    return resultRef;
}

// ------------------------------ JavaScriptCore/FunctionCallResolveNode ----------------------------------

RegisterID* FunctionCallResolveNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    llvm::Value *idRef = context.getVariableRef(m_ident.name, false);
    if (NULL == idRef)
    {
        idRef = context.getFunction(m_ident.name.c_str());
        if (NULL == idRef)
        {
            std::cout <<  __FUNCTION__ << " Can't find " << m_ident.name << "()" << std::endl;
            context.abort();
            return NULL;
        }
    }

    llvm::Value *param;
    ArgumentListNode* it;
    std::vector<llvm::Value *> args;
    llvm::Value *resultRef;
    for (it = m_args->m_listNode; it != NULL; it = it->next())
    {
        param = it->emitBytecode(context, dst);
        args.push_back(param);
    }
    
    if (llvm::isa<llvm::Function>(idRef))
    {
        resultRef = CodeGenJSObjectFunctionCall(context, (Function *)idRef, args);
    }
    else
    {
        llvm::Value *thisRef = context.getVariableRef(context.propertyNames->thisIdentifier.name);
        resultRef = CodeGenJSObjectConstructCall(context, idRef, thisRef, args);
    }

    std::cout <<  __FUNCTION__ << "Creating method call: " << m_ident.name << std::endl;

    return resultRef;
}


// ------------------------------ JavaScriptCore/FunctionCallBracketNode ----------------------------------

RegisterID* FunctionCallBracketNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == m_base) || (NULL == m_subscript))
    {
        std::cout <<  __FUNCTION__ << " m_base=" << m_base << ",m_subscript=" << m_subscript << std::endl;
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);
    RegisterID *iteratorRef = m_base->emitBytecode(context, dst);

    baseRef = CodeGenJSValue32ConstructIfNeeded(context, baseRef);
    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, iteratorRef);
    RegisterID *exprFunc = CodeGenJSValue32Construct(context, "");
    CodeGenJSValue32FunctionCopy(context, exprFunc, resultValue);

    //do it like FunctionCallResolveNode 
    RegisterID *param;
    ArgumentListNode* it;
    std::vector<RegisterID*> args;
    for (it = m_args->m_listNode; it != NULL; it = it->next())
    {
        param = ((ExpressionNode *)it)->emitBytecode(context, dst);
        args.push_back(param);
    }
    llvm::Value * newThisRef = baseRef;
    llvm::Value * resultRef = CodeGenJSObjectConstructCall(context, exprFunc, newThisRef, args);
    
    return resultRef;
}

// ------------------------------ JavaScriptCore/FunctionCallDotNode ----------------------------------

RegisterID* FunctionCallDotNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_base)
    {
        std::cout <<  __FUNCTION__ << " BAD FunctionCallDotNode" << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);
    
    baseRef = CodeGenJSValue32ConstructIfNeeded(context, baseRef);
    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, m_ident.name);
    RegisterID *exprFunc = CodeGenJSValue32Construct(context, m_ident.name.c_str());
    CodeGenJSValue32FunctionCopy(context, exprFunc, resultValue);

    //do it like FunctionCallResolveNode 
    RegisterID *param;
    ArgumentListNode* it;
    std::vector<RegisterID*> args;
    for (it = m_args->m_listNode; it != NULL; it = it->next())
    {
        param = ((ExpressionNode *)it)->emitBytecode(context, dst);
        args.push_back(param);
    }
    llvm::Value * newThisRef = baseRef;
    llvm::Value * resultRef = CodeGenJSObjectConstructCall(context, exprFunc, newThisRef, args);
    
    return resultRef;
}

RegisterID* CallFunctionCallDotNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_base)
    {
        std::cout <<  __FUNCTION__ << " BAD FunctionCallDotNode" << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);
    
    baseRef = CodeGenJSValue32ConstructIfNeeded(context, baseRef);
    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, m_ident.name);
    RegisterID *exprFunc = CodeGenJSValue32Construct(context, m_ident.name.c_str());
    CodeGenJSValue32FunctionCopy(context, exprFunc, resultValue);

    //do it like FunctionCallResolveNode 
    RegisterID *param;
    ArgumentListNode* it;
    std::vector<RegisterID*> args;
    for (it = m_args->m_listNode; it != NULL; it = it->next())
    {
        param = ((ExpressionNode *)it)->emitBytecode(context, dst);
        args.push_back(param);
    }
    llvm::Value * newThisRef = baseRef;
    llvm::Value * resultRef = CodeGenJSObjectConstructCall(context, exprFunc, newThisRef, args);
    
    return resultRef;
}

RegisterID* ApplyFunctionCallDotNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_base)
    {
        std::cout <<  __FUNCTION__ << " BAD FunctionCallDotNode" << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);
    
    baseRef = CodeGenJSValue32ConstructIfNeeded(context, baseRef);
    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, m_ident.name);
    RegisterID *exprFunc = CodeGenJSValue32Construct(context, m_ident.name.c_str());
    CodeGenJSValue32FunctionCopy(context, exprFunc, resultValue);

    //do it like FunctionCallResolveNode 
    RegisterID *param;
    ArgumentListNode* it;
    std::vector<RegisterID*> args;
    for (it = m_args->m_listNode; it != NULL; it = it->next())
    {
        param = it->emitBytecode(context, dst);
        args.push_back(param);
    }
    llvm::Value * newThisRef = baseRef;
    llvm::Value * resultRef = CodeGenJSObjectConstructCall(context, exprFunc, newThisRef, args);
    
    return resultRef;
}

// ------------------------------ JavaScriptCore/PostfixResolveNode ----------------------------------

RegisterID* PostfixResolveNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    RegisterID* idRef = context.getVariableRef(m_ident.name);
    if (NULL == idRef)
    {
        std::cout << __FUNCTION__ << " UNDECLARED " << m_ident.name << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID* tmpValue = NULL;
    switch (opcodeID())
    {
    case op_plusplus:
    case op_minusminus:
        tmpValue = CodeGenJSValue32FunctionOption(context, idRef, llvm::ConstantInt::getFalse(llvm::getGlobalContext()), opcodeID());
        break;

    default:
        std::cout <<  __FUNCTION__ << " UNIMPLEMENTED opcodeID=" << opcodeID() << std::endl;
        context.abort();
        break;
    }
    
    if (NULL == tmpValue)
    {
        return NULL;
    }

    RegisterID* resultRef = CodeGenJSValue32Construct(context, "");
    CodeGenJSValue32FunctionCopy(context, resultRef, idRef);
    CodeGenJSValue32FunctionCopy(context, idRef, tmpValue);

    return resultRef;
}

// ------------------------------ JavaScriptCore/PostfixBracketNode ----------------------------------

RegisterID* PostfixBracketNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == m_base) || (NULL == m_subscript))
    {
        std::cout <<  __FUNCTION__ << " m_base=" << m_base << ",m_subscript=" << m_subscript << std::endl;
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);
    RegisterID *iteratorRef = m_base->emitBytecode(context, dst);

    baseRef = CodeGenJSValue32ConstructIfNeeded(context, baseRef);
    iteratorRef = CodeGenJSValue32ConstructIfNeeded(context, iteratorRef);

    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, iteratorRef);

    RegisterID* tmpValue = NULL;
    switch (opcodeID())
    {
    case op_plusplus:
    case op_minusminus:
        tmpValue = CodeGenJSValue32FunctionOption(context, resultValue, llvm::ConstantInt::getFalse(llvm::getGlobalContext()), opcodeID());
        break;

    default:
        std::cout <<  __FUNCTION__ << " UNIMPLEMENTED opcodeID=" << opcodeID() << std::endl;
        context.abort();
        break;
    }

    if (NULL == tmpValue)
    {
        return NULL;
    }

    RegisterID* resultRef = CodeGenJSValue32Construct(context, "");
    CodeGenJSValue32FunctionCopy(context, resultRef, resultValue);
    CodeGenJSObjectSetPropertyCall(context, baseRef, iteratorRef, tmpValue);

    return resultRef;
}

// ------------------------------ JavaScriptCore/PostfixDotNode ----------------------------------

RegisterID* PostfixDotNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_base)
    {
        std::cout <<  __FUNCTION__ << " BAD AssignDotNode" << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);

    baseRef = CodeGenJSValue32ConstructIfNeeded(context, baseRef);
    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, m_ident.name);

    RegisterID* tmpValue = NULL;
    switch (opcodeID())
    {
    case op_plusplus:
    case op_minusminus:
        tmpValue = CodeGenJSValue32FunctionOption(context, resultValue, llvm::ConstantInt::getFalse(llvm::getGlobalContext()), opcodeID());
        break;

    default:
        std::cout <<  __FUNCTION__ << " UNIMPLEMENTED opcodeID=" << opcodeID() << std::endl;
        context.abort();
        break;
    }

    if (NULL == tmpValue)
    {
        return NULL;
    }

    RegisterID* resultRef = CodeGenJSValue32Construct(context, "");
    CodeGenJSValue32FunctionCopy(context, resultRef, resultValue);
    CodeGenJSObjectSetPropertyCall(context, baseRef, m_ident.name, tmpValue);

    return resultRef;
}

// ------------------------------ JavaScriptCore/PostfixErrorNode -----------------------------------

RegisterID* PostfixErrorNode::emitBytecode(CodeGenContext& context, RegisterID*)
{
    return emitThrowError(context, ReferenceError, opcodeID() == op_plusplus
        ? "Postfix ++ operator applied to value that is not a reference."
        : "Postfix -- operator applied to value that is not a reference.");
}

// ------------------------------ JavaScriptCore/DeleteResolveNode -----------------------------------

RegisterID* DeleteResolveNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    RegisterID* idRef = context.getVariableRef(m_ident.name);
    if (NULL == idRef)
    {
        std::cout << __FUNCTION__ << " UNDECLARED " << m_ident.name << std::endl;
        context.abort();
        return NULL;
    }

    return CodeGenJSValue32Destruct(context, idRef);
}

// ------------------------------ JavaScriptCore/DeleteBracketNode -----------------------------------

RegisterID* DeleteBracketNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == m_base) || (NULL == m_subscript))
    {
        std::cout <<  __FUNCTION__ << " m_base=" << m_base << ",m_subscript=" << m_subscript << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);
    RegisterID *iteratorRef = m_base->emitBytecode(context, dst);

    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, iteratorRef);

    return CodeGenJSValue32Destruct(context, resultValue);
}

// ------------------------------ JavaScriptCore/DeleteDotNode -----------------------------------

RegisterID* DeleteDotNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_base)
    {
        std::cout <<  __FUNCTION__ << " m_base=NULL" << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);

    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, m_ident.name);

    return CodeGenJSValue32Destruct(context, resultValue);
}

// ------------------------------ JavaScriptCore/DeleteValueNode -----------------------------------

RegisterID* DeleteValueNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_expr)
    {
        std::cout <<  __FUNCTION__ << " m_expr=NULL" << std::endl;
        context.abort();
        return NULL;
    }

    return CodeGenJSValue32Destruct(context, m_expr->emitBytecode(context, dst));
}

// ------------------------------ JavaScriptCore/VoidNode -------------------------------------

RegisterID* VoidNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_expr)
    {
        std::cout <<  __FUNCTION__ << " m_expr=NULL" << std::endl;
        context.abort();
        return NULL;
    }
    
    m_expr->emitBytecode(context, dst);
    return context.getVariableRef("_ejs_undefined");
}

// ------------------------------ JavaScriptCore/TypeOfValueNode -----------------------------------

RegisterID* TypeOfResolveNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    RegisterID* idRef = context.getVariableRef(m_ident.name);
    if (NULL == idRef)
    {
        idRef = context.getVariableRef("_ejs_undefined");
    }

    return CodeGenJSObjectTypeOfCall(context, idRef);
}

// ------------------------------ JavaScriptCore/TypeOfValueNode -----------------------------------

RegisterID* TypeOfValueNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_expr)
    {
        std::cout <<  __FUNCTION__ << " m_expr=NULL" << std::endl;
        context.abort();
        return NULL;
    }
    RegisterID* exprRef = m_expr->emitBytecode(context, dst);
    return CodeGenJSObjectTypeOfCall(context, exprRef);
}

// ------------------------------ JavaScriptCore/PrefixResolveNode ----------------------------------

RegisterID* PrefixResolveNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    RegisterID* idRef = context.getVariableRef(m_ident.name);
    if (NULL == idRef)
    {
        std::cout << __FUNCTION__ << " UNDECLARED " << m_ident.name << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID* tmpValue = NULL;
    switch (opcodeID())
    {
    case op_plusplus:
    case op_minusminus:
        tmpValue = CodeGenJSValue32FunctionOption(context, idRef, llvm::ConstantInt::getTrue(llvm::getGlobalContext()), opcodeID());
        break;

    default:
        std::cout <<  __FUNCTION__ << " UNIMPLEMENTED opcodeID=" << opcodeID() << std::endl;
        context.abort();
        break;
    }
    
    if (NULL != tmpValue)
    {
        CodeGenJSValue32FunctionCopy(context, idRef, tmpValue);
    }

    return tmpValue;
}

// ------------------------------ JavaScriptCore/PrefixBracketNode ----------------------------------

RegisterID* PrefixBracketNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == m_base) || (NULL == m_subscript))
    {
        std::cout <<  __FUNCTION__ << " m_base=" << m_base << ",m_subscript=" << m_subscript << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);
    RegisterID *iteratorRef = m_subscript->emitBytecode(context, dst);

    baseRef = CodeGenJSValue32ConstructIfNeeded(context, baseRef);
    iteratorRef = CodeGenJSValue32ConstructIfNeeded(context, iteratorRef);

    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, iteratorRef);

    RegisterID* tmpValue = NULL;
    switch (opcodeID())
    {
    case op_plusplus:
    case op_minusminus:
        tmpValue = CodeGenJSValue32FunctionOption(context, resultValue, llvm::ConstantInt::getTrue(llvm::getGlobalContext()), opcodeID());
        break;

    default:
        std::cout <<  __FUNCTION__ << " UNIMPLEMENTED opcodeID=" << opcodeID() << std::endl;
        context.abort();
        break;
    }

    if (NULL != tmpValue)
    {
        CodeGenJSObjectSetPropertyCall(context, baseRef,iteratorRef, tmpValue);
    }

    return tmpValue;
}

// ------------------------------ JavaScriptCore/PrefixDotNode ----------------------------------

RegisterID* PrefixDotNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_base)
    {
        std::cout <<  __FUNCTION__ << " BAD AssignDotNode" << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);

    RegisterID *resultValue = CodeGenJSObjectGetPropertyCall(context, baseRef, m_ident.name);

    RegisterID* tmpValue = NULL;
    switch (opcodeID())
    {
    case op_plusplus:
    case op_minusminus:
        tmpValue = CodeGenJSValue32FunctionOption(context, resultValue, llvm::ConstantInt::getTrue(llvm::getGlobalContext()), opcodeID());
        break;

    default:
        std::cout <<  __FUNCTION__ << " UNIMPLEMENTED opcodeID=" << opcodeID() << std::endl;
        context.abort();
        break;
    }
    
    if (NULL != tmpValue)
    {
        CodeGenJSObjectSetPropertyCall(context, baseRef, m_ident.name, tmpValue);
    }

    return tmpValue;
}

// ------------------------------ JavaScriptCore/PrefixErrorNode -----------------------------------

RegisterID* PrefixErrorNode::emitBytecode(CodeGenContext& context, RegisterID*)
{
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

// ------------------------------ JavaScriptCore/Unary Operation Nodes -----------------------------------

RegisterID* UnaryOpNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (op_not != opcodeID())
    {
        std::cout <<  __FUNCTION__ << " UNIMPLEMENTED opcodeID=" << opcodeID() << std::endl;
        context.abort();
        return NULL;
    }
    
    double constantValue;
    llvm::Value *condV = NULL;
    if (isConstant(constantValue))
    {
        if (constantValue)
        {
            // br(1) -> true.
            condV = llvm::ConstantInt::getTrue(llvm::getGlobalContext());
        }
        else
        {
            // br(0) -> false.
            condV = llvm::ConstantInt::getFalse(llvm::getGlobalContext());
        }
        return condV;
    }

    std::vector<LogicalInfo> logicalInfos;
    BasicBlock *currentBB = context.currentBlock();
    Function *currentFunction = currentBB->getParent();

    initLogicalInfo(logicalInfos, this, NULL, NULL);
    BasicBlock *endBB = logicalInfos[0].holdBB;

    llvm::Type *int1Type = llvm::Type::getInt1Ty(llvm::getGlobalContext());
    PHINode *phiNode = PHINode::Create(int1Type, 2, "", endBB);

    condV = EmitLogicalOptionExpr(context, logicalInfos, this, phiNode);
    bool needPHI = false;
    for (size_t index = 1; index < logicalInfos.size(); index++)
    {
        BasicBlock *indexBB = logicalInfos[index].holdBB;
        if (NULL == indexBB)
        {
            continue;
        }
        llvm::pred_iterator PI = pred_begin(indexBB);
        llvm::pred_iterator PE = pred_end(indexBB);
        if (PI == PE)
        {
            // if endBB has no incoming, remove it, just return condV as a UnaryOpNode
            delete indexBB;
        }
        else
        {
            needPHI = true;
            currentFunction->getBasicBlockList().push_back(indexBB);
        }
    }
    if (needPHI)
    {
        BranchInst::Create(endBB, context.currentBlock());
        phiNode->addIncoming(condV, context.currentBlock());
        currentFunction->getBasicBlockList().push_back(endBB);
        condV = phiNode;
        context.pushBlock(endBB);
    }
    else
    {
        delete endBB;
    }

    return condV;
}

// ------------------------------ JavaScriptCore/Binary Operation Nodes -----------------------------------

RegisterID* BinaryOpNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == m_expr1) || (NULL == m_expr2))
    {
        std::cout <<  __FUNCTION__ << " m_expr1=" << m_expr1 << ",m_expr2=" << m_expr2 << std::endl;
        context.abort();
        return NULL;
    }

    switch (opcodeID())
    {
    case op_add:
    case op_sub:
    case op_mul:
    case op_div:
    case op_mod:
        return CodeGenJSValue32FunctionOption(context, m_expr1->emitBytecode(context, dst), m_expr2->emitBytecode(context, dst), opcodeID());

    case op_eq:
    case op_neq:
    case op_less:
    case op_lesseq:
    case op_greater:
    case op_greatereq:
        return CodeGenJSValue32FunctionCmp(context, m_expr1->emitBytecode(context, dst), m_expr2->emitBytecode(context, dst), opcodeID());

    default:
        std::cout <<  __FUNCTION__ << " UNIMPLEMENTED opcodeID=" << opcodeID() << std::endl;
        context.abort();
        break;
    }

    return NULL;
}

RegisterID* EqualNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == m_expr1) || (NULL == m_expr2))
    {
        std::cout <<  __FUNCTION__ << " m_expr1=" << m_expr1 << ",m_expr2=" << m_expr2 << std::endl;
        context.abort();
        return NULL;
    }

    return CodeGenJSValue32FunctionCmp(context, m_expr1->emitBytecode(context, dst), m_expr2->emitBytecode(context, dst), op_eq);
}

RegisterID* StrictEqualNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == m_expr1) || (NULL == m_expr2))
    {
        std::cout <<  __FUNCTION__ << " m_expr1=" << m_expr1 << ",m_expr2=" << m_expr2 << std::endl;
        context.abort();
        return NULL;
    }

    return CodeGenJSValue32FunctionCmp(context, m_expr1->emitBytecode(context, dst), m_expr2->emitBytecode(context, dst), op_stricteq);
}

RegisterID* ReverseBinaryOpNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    //I'm not sure really need this--change a >= b to b <= a; change a > b to b < a
    //unless < and <= are more quick than > and >=
    if ((NULL == m_expr1) || (NULL == m_expr2))
    {
        std::cout <<  __FUNCTION__ << " m_expr1=" << m_expr1 << ",m_expr2=" << m_expr2 << std::endl;
        context.abort();
        return NULL;
    }

    switch (opcodeID())
    {
    case op_add:
    case op_sub:
    case op_mul:
    case op_div:
    case op_mod:
        return CodeGenJSValue32FunctionOption(context, m_expr1->emitBytecode(context, dst), m_expr2->emitBytecode(context, dst), opcodeID());

    case op_eq:
    case op_neq:
    case op_less:
    case op_lesseq:
    case op_greater:
    case op_greatereq:
        return CodeGenJSValue32FunctionCmp(context, m_expr1->emitBytecode(context, dst), m_expr2->emitBytecode(context, dst), opcodeID());

    default:
        std::cout <<  __FUNCTION__ << " UNIMPLEMENTED opcodeID=" << opcodeID() << std::endl;
        context.abort();
        break;
    }

    return NULL;
}

RegisterID* ThrowableBinaryOpNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

RegisterID* InstanceOfNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == m_expr1) || (NULL == m_expr2))
    {
        std::cout <<  __FUNCTION__ << " m_expr1=" << m_expr1 << ",m_expr2=" << m_expr2 << std::endl;
        context.abort();
        return NULL;
    }

    return CodeGenJSObjectInstanceOfCall(context, m_expr1->emitBytecode(context, dst), m_expr2->emitBytecode(context, dst));
}

// ------------------------------ JavaScriptCore/LogicalOpNode ----------------------------
/**
 * there are two case to use LogicalOpNode
 * if (a && b)
 *   so the upper block should set checktrueBB and checkfalseBB to decide which block to jump
 * v = a && b; somefunction(a || b)
 *   it's just a statement, nobody set checktrueBB and checkfalseBB for us, and the upper block just need the result of the check
 *   we should use PHINode to return just one value
 *   sure you can declare a varialbe, assign it and return...
 *
 * NOTICE, for nesting mode:
 * if (a && b || c && d .....) is easy to deal with
 *     because the real continue and end BasicBlocks are fixed by upper block
 * v = a && b || c && d ..... is hard to do
 *     there aren't really continue and end BasicBlocks, so the first nesting should mark the end BasicBlocks
 *     and the first nesting should mark it is in PHI mode too...
 *     I assume that: if ((checkfalseBB == NULL) && (checktrueBB != NULL)), it means in PHI nesting mode
 *
 * there are two methods to deal with nesting LogicalOpNode, 
 *
 * for example v = ((a && b) && c) :
 *   method I      method II
 * [?a        ]  [?a         ]
 * [?b        ]  [?b         ]
 * [?a&&b     ]
 * [?c        ]  [?c         ]
 * [?(a&&b)&&c]   [?(a&&b)&&c]
 *
 * another example v = (a && (b && c)) :
 *   method I      method II
 * [?a        ]  [?a         ]
 * [?b        ]  [?b         ]
 * [?c        ]  [?c         ]
 * [?b&&c     ]
 * [?a&&(b&&c)]   [?(a&&b)&&c] 
 *
 * there isn't a block to stroe temp result of [?a&&b     ] or  [?b&&c     ]  in method II, 
 * but they both need to record the real final block in first nesting
 * if LOGICALOP_SMART_NESTING is defined, use method II, or , use method I 
 */

RegisterID *LogicalOpNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    std::vector<LogicalInfo> logicalInfos;
    double constantValue;
	llvm::Value *condV = NULL;
    //something wrong with LOGICALOP_SMART_NESTING
    //console.log((1 && 9) && ((0 && 2) || (4 && 3 && 5)) && (6 || 7 && 8));
#define LOGICALOP_SMART_NESTING
    if ((NULL == m_expr1) || (NULL == m_expr2))
    {
        std::cout <<  __FUNCTION__ << " m_expr1=" << m_expr1 << ",m_expr2=" << m_expr2 << std::endl;
        context.abort();
        return NULL;
    }

    if (op_and != opcodeID() && (op_or != opcodeID()))
    {
        std::cout <<  __FUNCTION__ << " UNKNOWN opcodeID=" << opcodeID() << std::endl;
        context.abort();
        return NULL;
    }

#if defined(LOGICALOP_SMART_NESTING)
    // get blocks after condV
    BasicBlock *currentBB = context.currentBlock();
    BasicBlock *checktrueBB = context.getContinueBlock();
    BasicBlock *checkfalseBB = context.getBreakBlock();
    Function *currentFunction = currentBB->getParent();
    PHINode *phiNode = NULL;
    bool needPHI = false;
    if (NULL == currentFunction)
    {
        std::cout <<  __FUNCTION__ << " currentFunction=NULL" << std::endl;
        context.abort();
        return NULL;
    }
    
    if (((NULL == checktrueBB) && (NULL != checkfalseBB)) || ((NULL != checktrueBB) && (NULL == checkfalseBB)))
    {
        std::cout <<  __FUNCTION__ << " checktrueBB=" << checktrueBB << ",checkfalseBB=" << checkfalseBB << std::endl;
        context.abort();
        return NULL;
    }
    
    if (isConstant(constantValue))
    {
        if (constantValue)
        {
            // br(1) -> true.
            condV = llvm::ConstantInt::getTrue(llvm::getGlobalContext());
            if (NULL != checktrueBB)
            {
                BranchInst::Create(checktrueBB, context.currentBlock());
            }
        }
        else
        {
            // br(0) -> false.
            condV = llvm::ConstantInt::getFalse(llvm::getGlobalContext());
            if (NULL != checkfalseBB)
            {
                BranchInst::Create(checkfalseBB, context.currentBlock());
            }
        }
        return condV;
    }

    initLogicalInfo(logicalInfos, this, checktrueBB, checkfalseBB);
    
    BasicBlock *endBB = logicalInfos[0].holdBB;
    if ((NULL == checktrueBB) && (NULL == checkfalseBB))
    {
        llvm::Type *int1Type = llvm::Type::getInt1Ty(llvm::getGlobalContext());

        phiNode = PHINode::Create(int1Type, 2, "", endBB);
    }
    
    // if ((NULL != checktrueBB) && (NULL != checkfalseBB)), logicalInfos[0].holdBB will be NULL
    // if logicalInfos[0].holdBB is NULL, PHI will never use in this case
    condV = EmitLogicalOptionExpr(context, logicalInfos, this, phiNode);
    for (size_t index = 1; index < logicalInfos.size(); index++)
    {
        BasicBlock *indexBB = logicalInfos[index].holdBB;
        if (NULL == indexBB)
        {
            continue;
        }
        llvm::pred_iterator PI = pred_begin(indexBB);
        llvm::pred_iterator PE = pred_end(indexBB);
        if (PI == PE)
        {
            // if endBB has no incoming, remove it, just return condV as a UnaryOpNode
            delete indexBB;
        }
        else
        {
            needPHI = true;
            currentFunction->getBasicBlockList().push_back(indexBB);
        }
    }
    
    if (NULL == phiNode)
    {
        return condV;
    }
    else if (needPHI)
    {
        BranchInst::Create(endBB, context.currentBlock());
        phiNode->addIncoming(condV, context.currentBlock());
        currentFunction->getBasicBlockList().push_back(endBB);
        condV = phiNode;
        context.pushBlock(endBB);
    }
    else
    {
        delete endBB;
        return condV;
    }

    return condV;
#else
    // get blocks after condV
    Function *currentFunction = context.currentFunction();
    BasicBlock *checktrueBB = context.getContinueBlock();
    BasicBlock *checkfalseBB = context.getBreakBlock();
    int checkResult = 0;
    bool usePHI = false;
    bool firstNesting = false;
    if (NULL == currentFunction)
    {
        std::cout <<  __FUNCTION__ << " currentFunction=NULL" << std::endl;
        return NULL;
    }
    
    if (NULL == checktrueBB)
    {
        checktrueBB = BasicBlock::Create(getGlobalContext(), "checkend.real", currentFunction);
        context.setContinueBlock(checktrueBB);
        usePHI = true;
        firstNesting = true;
    }

    // do it before create BasicBlock, since it may create BasicBlock also...
    llvm::Value *condV = CodeGenJSValue32FunctionIsTrue(context, m_expr1->emitBytecode(context, dst));
    //currentBB may change atfer m_expr1->emitBytecode
    BasicBlock *currentBB = context.currentBlock();

    BasicBlock *checkcontBB = BasicBlock::Create(getGlobalContext(), "checkcont", currentFunction, checktrueBB);
    if (NULL == checkfalseBB)
    {
        if (!usePHI)
        {
            checktrueBB = BasicBlock::Create(getGlobalContext(), "checkend", currentFunction, checktrueBB);
            usePHI = true;
        }
        checkfalseBB = checktrueBB;
    }

    switch (opcodeID())
    {
    case op_and:
        BranchInst::Create(checkcontBB, checkfalseBB, condV, context.currentBlock());
        checkResult = 0;
        break;

    case op_or:
        BranchInst::Create(checktrueBB, checkcontBB, condV, context.currentBlock());
        checkResult = 1;
        break;

    default:
        std::cout <<  __FUNCTION__ << " UNIMPLEMENTED opcodeID=" << opcodeID() << std::endl;
        break;
    }

    context.pushBlock(checkcontBB);
    condV = CodeGenJSValue32FunctionIsTrue(context, m_expr2->emitBytecode(context, dst));
    //checkcontBB may change atfer m_expr2->emitBytecode
    checkcontBB = context.currentBlock();
    if (usePHI)
    {
        BranchInst::Create(checktrueBB, context.currentBlock());
        context.pushBlock(checktrueBB);
        
        llvm::Type *int1Type = llvm::Type::getInt1Ty(llvm::getGlobalContext());
        llvm::Type *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
        PHINode *phiNode = PHINode::Create(int1Type, 2, "", checktrueBB);

        phiNode->addIncoming(ConstantInt::get(int1Type, checkResult), currentBB);
        phiNode->addIncoming(condV, checkcontBB);

        if (firstNesting)
        {
            context.setContinueBlock(NULL);
        }
        return phiNode;
    }

    return BranchInst::Create(checktrueBB, checkfalseBB, condV, context.currentBlock());
#endif
}

// ------------------------------ JavaScriptCore/ConditionalNode ------------------------------

RegisterID* ConditionalNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    // (cond) ? expr1 : expr2;
    // Output this as:
    //   ...
    //   cond
    //   cond & br cond.true cond.false
    // cond.true: 
    //   expr1
    //   br cond.end
    // cond.false: 
    //   expr2
    //   br cond.end
    // cond.end:
    //   ...
    if ((NULL == m_logical) || (NULL == m_expr1) || (NULL == m_expr2))
    {
        std::cout <<  __FUNCTION__ << " m_logical=" << m_logical << ",m_expr1=" << m_expr1 << ",m_expr2=" << m_expr2 << std::endl;
        context.abort();
        return NULL;
    }
    
    llvm::Value *tempRef = dst;
    if (NULL == tempRef)
    {
        tempRef = CodeGenJSValue32Construct(context, "");
    }

    BasicBlock *trueBB = BasicBlock::Create(getGlobalContext(), "cond.true");
    BasicBlock *falseBB = BasicBlock::Create(getGlobalContext(), "cond.false");
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "cond.end");

    context.setContinueBlock(trueBB);
    context.setBreakBlock(falseBB);
    RegisterID *condV = m_logical->emitBytecode(context, dst);
    if (NULL == condV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for condition" << std::endl;
        return NULL;
    }
    context.setContinueBlock(NULL);
    context.setBreakBlock(NULL);

    //check whether condtrueBB & condfalseBB are in use,
    //if not, means that, condV isn't a real condV with BranchInst
    llvm::pred_iterator PI = pred_begin(trueBB);
    llvm::pred_iterator PE = pred_end(trueBB);
    if (PI == PE)
    {
        condV = CodeGenJSValue32FunctionIsTrue(context, condV);
        BranchInst::Create(trueBB, falseBB, condV, context.currentBlock());
    }

    Function *currentFunction = context.currentFunction();

    context.pushBlock(trueBB);
    currentFunction->getBasicBlockList().push_back(trueBB);
    RegisterID *ifV = m_expr1->emitBytecode(context, dst);
    if (NULL == ifV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for m_expr1" << std::endl;
        return NULL;
    }
    CodeGenJSValue32FunctionCopy(context, tempRef, ifV);
    BranchInst::Create(endBB, trueBB);

    context.pushBlock(falseBB);
    currentFunction->getBasicBlockList().push_back(falseBB);
    RegisterID *elseV = m_expr2->emitBytecode(context, dst);
    if (NULL == elseV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for m_expr2" << std::endl;
        return NULL;
    }
    CodeGenJSValue32FunctionCopy(context, tempRef, elseV);
    BranchInst::Create(endBB, falseBB);

    context.pushBlock(endBB);
    currentFunction->getBasicBlockList().push_back(endBB);

    return tempRef;
}

// ------------------------------ JavaScriptCore/ReadModifyResolveNode -----------------------------------

RegisterID* ReadModifyResolveNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    RegisterID* leftRef = context.getVariableRef(m_ident.name);
    if (NULL == leftRef)
    {
        std::cout << __FUNCTION__ << " UNDECLARED " << m_ident.name << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID* rightRef = NULL;
    if (NULL != m_right)
    {
        rightRef = m_right->emitBytecode(context, dst);
    }
    OpcodeID option = opcodeID();
    switch (opcodeID())
    {
    case op_eq:
        std::cout << __FUNCTION__ << " Should use AssignResolveNode instead of ReadModifyResolveNode" << std::endl;
        break;
        
    case op_pluseq:
        option = op_add;
        break;
    
    case op_minuseq:
        option = op_sub;
        break;
    
    case op_multeq:
        option = op_mul;
        break;
    
    case op_diveq:
        option = op_div;
        break;

    case op_andeq:
        option = op_bitand;
        break;

    case op_xoreq:
        option = op_bitxor;
        break;

    case op_oreq:
        option = op_bitor;
        break;

    case op_modeq:
        option = op_mod;
        break;

    case op_lshift:
    case op_rshift:
    case op_urshift:
        break;

    default:
        std::cout <<  __FUNCTION__ << " UNIMPLEMENTED opcodeID=" << opcodeID() << std::endl;
        context.abort();
        option = op_end;
        break;
    }

    RegisterID* tmpValue = NULL;
    if (op_end != option)
    {
        RegisterID* tmpValue = CodeGenJSValue32FunctionOption(context, leftRef, rightRef, option);
        CodeGenJSValue32FunctionCopy(context, leftRef, tmpValue);
    }

    return leftRef;
}

// ------------------------------ JavaScriptCore/AssignResolveNode -----------------------------------

RegisterID* AssignResolveNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    //if AssignResolveNode is started with a var, the first access of it should be VarStatementNode::emitBytecode
    //so if can't find a idnet when assign it, treat it as global variable
    const std::string &name = identifier().name;
    if (context.isConstVariable(name))
    {
        std::cout <<  __FUNCTION__ << " Can't change the value of constant varialbe: " << name << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID* idRef = context.getVariableRef(name);
    if (NULL == idRef)
    {
        idRef = createVariable(context, false);
    }

    RegisterID* value = m_right->emitBytecode(context, idRef);
    CodeGenJSValue32FunctionCopy(context, idRef, value);

    return idRef;
}

// ------------------------------ JavaScriptCore/AssignDotNode -----------------------------------

RegisterID* AssignDotNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == m_base) || (NULL == m_right))
    {
        std::cout <<  __FUNCTION__ << " BAD AssignDotNode:m_base=" << m_base << ",m_right=" << m_right << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *rightValue = m_right->emitBytecode(context, dst);
    RegisterID *baseRef = m_base->emitBytecode(context, dst);

    return CodeGenJSObjectSetPropertyCall(context, baseRef, m_ident.name, rightValue);
}

// ------------------------------ JavaScriptCore/ReadModifyDotNode -----------------------------------

RegisterID* ReadModifyDotNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

// ------------------------------ JavaScriptCore/AssignErrorNode -----------------------------------

RegisterID* AssignErrorNode::emitBytecode(CodeGenContext& context, RegisterID*)
{
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

// ------------------------------ JavaScriptCore/AssignBracketNode -----------------------------------

RegisterID* AssignBracketNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if ((NULL == m_base) || (NULL == m_subscript))
    {
        std::cout <<  __FUNCTION__ << " m_base=" << m_base << ",m_subscript=" << m_subscript << std::endl;
        context.abort();
        return NULL;
    }

    RegisterID *baseRef = m_base->emitBytecode(context, dst);
    RegisterID *iteratorRef = m_subscript->emitBytecode(context, dst);
    RegisterID *valueRef = m_right->emitBytecode(context, dst);

    RegisterID *resultValue = CodeGenJSObjectSetPropertyCall(context, baseRef, iteratorRef, valueRef);

    return resultValue;
}

// ------------------------------ JavaScriptCore/ReadModifyBracketNode -----------------------------------

RegisterID* ReadModifyBracketNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

// ------------------------------ JavaScriptCore/CommaNode ------------------------------------

RegisterID* CommaNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    ExpressionVector::const_iterator it;
    for (it = m_expressions.begin(); it != m_expressions.end(); it++)
    {
        (*it)->emitBytecode(context, dst);
    }

    return NULL;
}

// ------------------------------ JavaScriptCore/ConstDeclNode ------------------------------------
RegisterID* ConstDeclNode::createConstant(CodeGenContext &context, bool inLocal)
{
    llvm::Function *currentFunction = context.currentFunction();
    llvm::Function *mainFunction = context.mainFunction();
    RegisterID *idRef = NULL;
    
    // if the variable is create in main, create as global
    if (!inLocal || (currentFunction == mainFunction))
    {
        llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
        idRef = new GlobalVariable(*context.getModule(), jsValueType, false, GlobalValue::ExternalLinkage, 0, m_ident.name);
        context.setVariableRef(m_ident.name, idRef, false, true);
    }
    else
    {
        idRef = CodeGenJSValue32Construct(context, m_ident.name.c_str());
        context.setVariableRef(m_ident.name, idRef, true, true);
    }

    return idRef;
}

RegisterID* ConstDeclNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    llvm::Function *currentFunction = context.currentFunction();
    llvm::Function *mainFunction = context.mainFunction();
    llvm::Value *idRef = context.getVariableRef(m_ident.name, (currentFunction != mainFunction));
    if (NULL != idRef)
    {
        std::cout <<  __FUNCTION__ << " Constant " << idRef << " already exists" << std::endl;
        context.abort();
        return NULL;
    }
    idRef = createConstant(context);
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    if (context.isGlobalVariable(m_ident.name))
    {
        llvm::Constant *initValue = ConstantAggregateZero::get(jsValueType);
        if (NULL != m_init)
        {
            double constantValue;
            if (m_init->isConstant(constantValue))
            {
                llvm::Type *int64Type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
                ejsval jsValue;
                jsValue.asDouble = constantValue;
                initValue = ConstantInt::get(int64Type, jsValue.asInt64);
            }
            else
            {
                std::cout <<  __FUNCTION__ << " Unkonw initializer opcodeID=" << m_init->opcodeID() << std::endl;
            }
        }
        ((GlobalVariable *)idRef)->setInitializer(initValue);
    }
    else if (NULL != m_init)
    {
        CodeGenJSValue32FunctionCopy(context, idRef, m_init->emitBytecode(context, dst));
    }

    return idRef;
}

// ------------------------------ JavaScriptCore/ConstStatementNode -----------------------------

RegisterID* ConstStatementNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    ConstDeclNode *index;
    for (index = m_list; NULL != index; index = index->next())
    {
        index->emitBytecode(context, dst);
    }
    
    return NULL;
}

// ------------------------------ JavaScriptCore/SourceElements -------------------------------

RegisterID* SourceElements::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    std::vector<StatementNode*>::const_iterator it;
    RegisterID *last = NULL;
    for (it = m_statements.begin(); it != m_statements.end(); it++)
    {
        last = (*it)->emitBytecode(context, dst);
    }

    return last;
}

// ------------------------------ JavaScriptCore/BlockNode ------------------------------------

inline StatementNode* BlockNode::lastStatement() const
{
    return m_statements ? m_statements->lastStatement() : 0;
}

RegisterID* BlockNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_statements)
    {
        std::cout <<  __FUNCTION__ << " m_statements=NULL" << std::endl;
        context.abort();
        return NULL;
    }

    return m_statements->emitBytecode(context, dst);
}

// ------------------------------ JavaScriptCore/EmptyStatementNode ---------------------------

RegisterID* EmptyStatementNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

// ------------------------------ JavaScriptCore/DebuggerStatementNode ---------------------------

RegisterID* DebuggerStatementNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

// ------------------------------ JavaScriptCore/ExprStatementNode ----------------------------

RegisterID* ExprStatementNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_expr)
    {
        std::cout <<  __FUNCTION__ << " m_expr=NULL" << std::endl;
        context.abort();
        return NULL;
    }
    return m_expr->emitBytecode(context, dst);
}

// ------------------------------ JavaScriptCore/VarStatementNode ----------------------------

RegisterID* VarStatementNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    //var ident[ = value][;]
    // ident should be exclusive in each scope, but sub scope can declare a ident even the parent scope already has a variable with the same name, if the ident is declared with "var"
    if (NULL == m_expr)
    {
        std::cout <<  __FUNCTION__ << " m_expr=NULL" << std::endl;
        context.abort();
        return NULL;
    }
    llvm::Function *currentFunction = context.currentFunction();
    llvm::Function *mainFunction = context.mainFunction();
    llvm::Value *idRef;
    OpcodeID opcodeID = m_expr->opcodeID();
    if (op_resolve == opcodeID)
    {
        //check whether ident is existed in this scope
        ResolveNode *resolveExpr = (ResolveNode *)m_expr;
        const Identifier& ident = resolveExpr->identifier();
        idRef = context.getVariableRef(ident.name, (currentFunction != mainFunction));
        if (NULL != idRef)
        {
            std::cout <<  __FUNCTION__ << " variable " << idRef << " already exists" << std::endl;
            context.abort();
            return NULL;
        }
        idRef = resolveExpr->createVariable(context);
    }
    else if (op_resolve_list == opcodeID)
    {
        CommaNode *commaExpr = (CommaNode *)m_expr;
        ExpressionVector &exprList = commaExpr->list();
        ExpressionVector::const_iterator it;

        for (it = exprList.begin(); it != exprList.end(); it++)
        {
            if (op_resolve != (*it)->opcodeID())
            {
                std::cout <<  __FUNCTION__ << "BAD ResolveNode for CommaNode" << std::endl;
                context.abort();
                return NULL;
            }
            ResolveNode *resolveExpr = (ResolveNode *)(*it);
            const Identifier& ident = resolveExpr->identifier();
            idRef = context.getVariableRef(ident.name, (currentFunction != mainFunction));
            if (NULL != idRef)
            {
                std::cout <<  __FUNCTION__ << " variable " << idRef << " already exists" << std::endl;
                context.abort();
                return NULL;
            }
            idRef = resolveExpr->createVariable(context);
        }
    }
    else
    {
        std::cout <<  __FUNCTION__ << "BAD ResolveNode for VarStatementNode" << std::endl;
        context.abort();
        return NULL;
    }

    //do the real emitBytecode of expression, ex, AssignResolveNode
    return m_expr->emitBytecode(context, dst);
}

// ------------------------------ JavaScriptCore/IfNode ---------------------------------------

RegisterID* IfNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    // if (excond)
    //    exif
    // Output this as:
    //   ...
    //   excond
    //   cond & br cond.true cond.end
    // cond.true: 
    //   exif
    //   br cond.end
    // cond.end:
    //   ...
    if ((NULL == m_condition) || (NULL == m_ifBlock))
    {
        std::cout <<  __FUNCTION__ << " m_condition=" << m_condition << ",m_ifBlock=" << m_ifBlock << std::endl;
        context.abort();
        return NULL;
    }

    Function *currentFunction = context.currentFunction();
    BasicBlock *trueBB = BasicBlock::Create(getGlobalContext(), "cond.true");
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "cond.end");

    context.setContinueBlock(trueBB);
    context.setBreakBlock(endBB);
    RegisterID *condV = m_condition->emitBytecode(context, dst);
    if (NULL == condV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for condition" << std::endl;
        context.abort();
        return NULL;
    }
    context.setContinueBlock(NULL);
    context.setBreakBlock(NULL);

    //check whether condtrueBB & condfalseBB are in use,
    //if not, means that, condV isn't a real condV with BranchInst
    llvm::pred_iterator PI = pred_begin(trueBB);
    llvm::pred_iterator PE = pred_end(trueBB);
    if (PI == PE)
    {
        condV = CodeGenJSValue32FunctionIsTrue(context, condV);
        BranchInst::Create(trueBB, endBB, condV, context.currentBlock());
    }

    context.pushBlock(trueBB);
    currentFunction->getBasicBlockList().push_back(trueBB);
    RegisterID *ifV = m_ifBlock->emitBytecode(context, dst);
    if (NULL == ifV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for m_ifBlock" << std::endl;
        context.abort();
        return NULL;
    }
    if (NULL == context.currentBlock()->getTerminator())
    {
        BranchInst::Create(endBB, context.currentBlock());
    }
    context.popBlock(trueBB);

    context.pushBlock(endBB);
    currentFunction->getBasicBlockList().push_back(endBB);

    return NULL;
#if 0
    IRBuilder<> builder(context.currentBlock());
    RegisterID *condV = m_condition->emitBytecode(context, dst);
    if (NULL == condV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for condition" << std::endl;
        return NULL;
    }
    
    // Convert condition to a bool by comparing equal to 0.0.
    condV = builder.CreateFCmpONE(condV, ConstantFP::get(getGlobalContext(), APFloat(0.0)), "ifcond");
    
    Function *currentFunction = context.currentBlock()->getParent();
  
    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    BasicBlock *ifBB = BasicBlock::Create(getGlobalContext(), "if", currentFunction);
    BasicBlock *elseBB = BasicBlock::Create(getGlobalContext(), "else");
    BasicBlock *ifcontBB = BasicBlock::Create(getGlobalContext(), "ifcont");
    
    context.pushBlock(ifBB);
    builder.CreateCondBr(condV, ifBB, elseBB);
    
    // Emit then value.
    builder.SetInsertPoint(ifBB);
    
    RegisterID *ifV = m_ifBlock->emitBytecode(context, dst);
    if (NULL == ifV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for then" << std::endl;
        return NULL;
    }

    builder.CreateBr(ifcontBB);
    // Codegen of 'Then' can change the current block, update ifBB for the PHI.
    ifBB = builder.GetInsertBlock();
    
    context.popBlock();
    
    // Emit else block.
    context.pushBlock(elseBB);
    
    currentFunction->getBasicBlockList().push_back(elseBB);
    builder.SetInsertPoint(elseBB);
    
    RegisterID *elseV = m_elseBlock->emitBytecode(context, dst);
    if (NULL == elseV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for else" << std::endl;
        return NULL;
    }
    
    builder.CreateBr(ifcontBB);
    // Codegen of 'Else' can change the current block, update elseBB for the PHI.
    elseBB = builder.GetInsertBlock();
    
    context.popBlock();

    // Emit ifcont block.
    context.pushBlock(ifcontBB);
    
    currentFunction->getBasicBlockList().push_back(ifcontBB);
    builder.SetInsertPoint(ifcontBB);
    PHINode *phiNode = builder.CreatePHI(Type::getDoubleTy(getGlobalContext()), 2, "iftmp");
    
    phiNode->addIncoming(ifV, ifBB);
    phiNode->addIncoming(elseV, elseBB);

    /**
     * the code blow should follow ifcont, instead of last block such as "entry"
     * so, do popBlock when function is parsed completly
     */
    //context.popBlock();

    return phiNode;
#endif
}

// ------------------------------ JavaScriptCore/IfElseNode ---------------------------------------

RegisterID* IfElseNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    // if (excond)
    //    exif
    // else
    //    exelse
    // Output this as:
    //   ...
    //   excond
    //   cond & br cond.true cond.false
    // cond.true: 
    //   exif
    //   br cond.end
    // cond.false: 
    //   exelse
    //   br cond.end
    // [cond.end]:
    //   ...
    if ((NULL == m_condition) || (NULL == m_ifBlock) || (NULL == m_elseBlock))
    {
        std::cout <<  __FUNCTION__ << " m_condition=" << m_condition << ",m_ifBlock=" << m_ifBlock << ",m_elseBlock=" << m_elseBlock << std::endl;
        context.abort();
        return NULL;
    }

    Function *currentFunction = context.currentFunction();
    BasicBlock *trueBB = BasicBlock::Create(getGlobalContext(), "cond.true");
    BasicBlock *falseBB = BasicBlock::Create(getGlobalContext(), "cond.false");
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "cond.end");
    bool needEndBB = false;

    context.setContinueBlock(trueBB);
    context.setBreakBlock(falseBB);
    RegisterID *condV = m_condition->emitBytecode(context, dst);
    if (NULL == condV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for condition" << std::endl;
        context.abort();
        return NULL;
    }
    context.setContinueBlock(NULL);
    context.setBreakBlock(NULL);

    //check whether condtrueBB & condfalseBB are in use,
    //if not, means that, condV isn't a real condV with BranchInst
    llvm::pred_iterator PI = pred_begin(trueBB);
    llvm::pred_iterator PE = pred_end(trueBB);
    if (PI == PE)
    {
        condV = CodeGenJSValue32FunctionIsTrue(context, condV);
        BranchInst::Create(trueBB, falseBB, condV, context.currentBlock());
    }

    context.pushBlock(trueBB);
    currentFunction->getBasicBlockList().push_back(trueBB);
    RegisterID *ifV = m_ifBlock->emitBytecode(context, dst);
    if (NULL == ifV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for m_ifBlock" << std::endl;
        //context.abort();
        //return NULL;
    }
    if (NULL == context.currentBlock()->getTerminator())
    {
        BranchInst::Create(endBB, context.currentBlock());
        needEndBB = true;
    }
    context.popBlock(trueBB);

    context.pushBlock(falseBB);
    currentFunction->getBasicBlockList().push_back(falseBB);
    RegisterID *elseV = m_elseBlock->emitBytecode(context, dst);
    if (NULL == elseV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for m_elseBlock" << std::endl;
        //context.abort();
        //return NULL;
    }
    if (NULL == context.currentBlock()->getTerminator())
    {
        BranchInst::Create(endBB, context.currentBlock());
        needEndBB = true;
    }

    if (needEndBB)
    {
        context.popBlock(falseBB);
        
        context.pushBlock(endBB);
        currentFunction->getBasicBlockList().push_back(endBB);
    }
    else
    {
        delete endBB;
    }

    return NULL;
}

// ------------------------------ JavaScriptCore/DoWhileNode ----------------------------------

RegisterID* DoWhileNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    // do 
    //    exbody
    // while (ex1)
    // Output this as:
    //   ...
    //   br while.true
    // while.check: 
    //   ex1
    //   cond & br while.true, while.end
    // while.true: 
    //   exbody
    //   br while.check
    // [while.end]:
    //   ...
    if ((NULL == m_expr) || (NULL == m_statement))
    {
        std::cout <<  __FUNCTION__ << " m_expr=" << m_expr << ",m_statement=" << m_statement << std::endl;
        context.abort();
        return NULL;
    }

    Function *currentFunction = context.currentFunction();
    BasicBlock *checkBB = BasicBlock::Create(getGlobalContext(), "while.check", currentFunction);
    BasicBlock *trueBB = BasicBlock::Create(getGlobalContext(), "while.true");
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "while.end");

    BranchInst::Create(trueBB, context.currentBlock());
    context.pushBlock(checkBB);

    context.setContinueBlock(trueBB);
    context.setBreakBlock(endBB);
    RegisterID *condV = m_expr->emitBytecode(context, dst);
    if (NULL == condV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for condition" << std::endl;
        context.abort();
        return NULL;
    }

    //check whether condtrueBB & condfalseBB are in use,
    //if not, means that, condV isn't a real condV with BranchInst
    llvm::pred_iterator PI = pred_begin(trueBB);
    llvm::pred_iterator PE = pred_end(trueBB);
    if (PI == PE)
    {
        condV = CodeGenJSValue32FunctionIsTrue(context, condV);
        BranchInst::Create(trueBB, endBB, condV, context.currentBlock());
    }

    context.pushBlock(trueBB);
    currentFunction->getBasicBlockList().push_back(trueBB);
    m_statement->emitBytecode(context, dst);
    if (NULL == context.currentBlock()->getTerminator())
    {
        BranchInst::Create(checkBB, context.currentBlock());
    }

    context.setContinueBlock(NULL);
    context.setBreakBlock(NULL);
    context.pushBlock(endBB);
    currentFunction->getBasicBlockList().push_back(endBB);

    return NULL;
}

// ------------------------------ JavaScriptCore/WhileNode ------------------------------------

RegisterID* WhileNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    // while (ex1)
    //    exbody
    // Output this as:
    //   ...
    //   br while.check
    // while.check: 
    //   ex1
    //   cond & br while.true, while.end
    // while.true: 
    //   exbody
    //   br while.check
    // [while.end]:
    //   ...
    
    if ((NULL == m_expr) || (NULL == m_statement))
    {
        std::cout <<  __FUNCTION__ << " m_expr=" << m_expr << ",m_statement=" << m_statement << std::endl;
        context.abort();
        return NULL;
    }

    Function *currentFunction = context.currentFunction();
    BasicBlock *checkBB = BasicBlock::Create(getGlobalContext(), "while.check", currentFunction);
    BasicBlock *trueBB = BasicBlock::Create(getGlobalContext(), "while.true");
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "while.end");

    BranchInst::Create(checkBB, context.currentBlock());
    context.pushBlock(checkBB);

    context.setContinueBlock(trueBB);
    context.setBreakBlock(endBB);
    RegisterID *condV = m_expr->emitBytecode(context, dst);
    if (NULL == condV)
    {
        std::cout <<  __FUNCTION__ << " Error when GenCode for condition" << std::endl;
        return NULL;
    }

    //check whether condtrueBB & condfalseBB are in use,
    //if not, means that, condV isn't a real condV with BranchInst
    llvm::pred_iterator PI = pred_begin(trueBB);
    llvm::pred_iterator PE = pred_end(trueBB);
    if (PI == PE)
    {
        condV = CodeGenJSValue32FunctionIsTrue(context, condV);
        BranchInst::Create(trueBB, endBB, condV, context.currentBlock());
    }

    context.setContinueBlock(checkBB);
    context.pushBlock(trueBB);
    currentFunction->getBasicBlockList().push_back(trueBB);
    m_statement->emitBytecode(context, dst);
    if (NULL == context.currentBlock()->getTerminator())
    {
        BranchInst::Create(checkBB, context.currentBlock());
    }

    context.setContinueBlock(NULL);
    context.setBreakBlock(NULL);
    context.pushBlock(endBB);
    currentFunction->getBasicBlockList().push_back(endBB);

    return NULL;
}

// ------------------------------ JavaScriptCore/ForNode --------------------------------------

RegisterID* ForNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    // for (ex1; ex2; ex3)
    //    exbody
    // Output this as:
    //   ...
    //   ex1
    //   br for.check
    // for.check: 
    //   ex2
    // for: 
    //   exbody
    // for.step: 
    //   ex3
    //   cond & br loop
    // [for.end]:
    //   ...
    
    Function *currentFunction = context.currentFunction();
  
    // Make the new basic block for the loop header, inserting after current block.
    BasicBlock *loopcheckBB = BasicBlock::Create(getGlobalContext(), "for.check", currentFunction);
    BasicBlock *loopBB = BasicBlock::Create(getGlobalContext(), "for", currentFunction);
    BasicBlock *loopstepBB = BasicBlock::Create(getGlobalContext(), "for.step", currentFunction);
    BasicBlock *loopendBB = BasicBlock::Create(getGlobalContext(), "for.end", currentFunction);
    
    // Create an alloca for the variable in the entry block.
    if (NULL != m_expr1)
    {
        m_expr1->emitBytecode(context, dst);
    }

    //create a jmp to loopcheckBB to pass loopBB->getTerminator() check
    BranchInst::Create(loopcheckBB, context.currentBlock());
    
    context.setBreakBlock(loopendBB);
    context.setContinueBlock(loopstepBB);
    context.pushBlock(loopcheckBB);
    RegisterID *condV = NULL;
    if (NULL != m_expr2)
    {
        condV = m_expr2->emitBytecode(context, dst);
        condV = CodeGenJSValue32FunctionIsTrue(context, condV);
    }
    BranchInst::Create(loopBB, loopendBB, condV, context.currentBlock());
    context.popBlock(loopcheckBB);
    
    context.pushBlock(loopBB);
    if (NULL != m_statement)
    {
        m_statement->emitBytecode(context, dst);
    }
    if (NULL == context.currentBlock()->getTerminator())
    {
        BranchInst::Create(loopstepBB, context.currentBlock());
    }
    context.popBlock(loopBB);

    context.pushBlock(loopstepBB);
    if (NULL != m_expr3)
    {
        m_expr3->emitBytecode(context, dst);
    }
    if (NULL == context.currentBlock()->getTerminator())
    {
        BranchInst::Create(loopcheckBB, context.currentBlock());
    }
    context.popBlock(loopstepBB);
    
    context.pushBlock(loopendBB);
    /**
     * the code blow should follow whileendBB, instead of last block such as "entry"
     * so, do popBlock when function is parsed completly
     */
    //context.popBlock();

    // for expr always returns 0.0.
    return Constant::getNullValue(Type::getDoubleTy(getGlobalContext()));
}

// ------------------------------ JavaScriptCore/ForInNode ------------------------------------

RegisterID* ForInNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    // for ([var] m_lexpr [= m_init] in m_expr)
    //    m_statement
    //
    // [for in] is so complex that I have to write some pseudocode:
    //
    // idRef = m_lexpr;
    // arrayRef = m_expr;
    // bool isArray = _ejs_op_typeof_is_array(arrayRef);
    // if (!isArray)
    // {
    //     arrayRef = _ejs_Object_getOwnPropertyNames(_ejs_undefined, _ejs_undefined, 1, &arrayRef);
    // }
    // jsLength = _ejs_specop_get(arrayRef, _ejs_atom_length, arrayRef);
    // length = (int)ToUint32(jsLength);
    // 
    // ejsval arrayItem;
    // for (index = 0; index < length; index++)
    // {
    //     idRef = INT32_TO_EJSVAL_IMPL(index);
    //     if (!isArray)
    //     {
    //         idRef = _ejs_specop_get(arrayRef, INT32_TO_EJSVAL_IMPL(index), arrayRef);
    //     }
    //     m_statement;
    // }
    //
    // Output this as:
    //     idRef = m_lexpr
    //     arrayRef = m_expr
    //     isArray = _ejs_op_typeof_is_array(arrayRef);
    //     isArray ? branch forin.asarray, forin.asobject
    // forin.asobject
    //     arrayRef = _ejs_Object_getOwnPropertyNames(_ejs_undefined, _ejs_undefined, 1, &arrayRef);
    //     branch forin.asarray
    // forin.asarray:
    //     jsLength = _ejs_specop_get(arrayRef, _ejs_atom_length, arrayRef);
    //     length = (int)ToUint32(jsLength);
    //     branch forin.check
    // forin.check
    //     (index<length) ? branch forin.body.asarray, forin.end
    // forin.body.asarray
    //     idRef = INT32_TO_EJSVAL_IMPL(index);
    //     isArray ? branch forin.body, forin.body.asobject
    // forin.body.asobject
    //     idRef = _ejs_specop_get(arrayRef, idRef, arrayRef);
    //     branch forin.body
    // forin.body
    //     m_statement;
    //     branch forin.step
    // forin.step
    //     index++;
    //     branch forin.check
    // forin.end
    //   ...
    if ((NULL == m_expr) || (NULL == m_statement))
    {
        std::cout <<  __FUNCTION__ << " m_expr=" << m_expr << ",m_statement=" << m_statement << std::endl;
        context.abort();
        return NULL;
    }
    
    //     idRef = m_lexpr
    llvm::Value *idRef = NULL;
    if (NULL != m_init)
    {
        idRef = m_init->emitBytecode(context, dst);
    }
    else if (NULL != m_lexpr)
    {
        idRef = m_lexpr->emitBytecode(context, dst);
    }
    else
    {
        idRef = context.getVariableRef(m_ident.name);
    }
    //     arrayRef = m_expr
    llvm::Value *exprRef = m_expr->emitBytecode(context, dst);
    llvm::Value *jsArrayRef = CodeGenJSValue32Construct(context, "");
    CodeGenJSValue32FunctionCopy(context, jsArrayRef, exprRef);

    //     isArray = _ejs_op_typeof_is_array(arrayRef);
    Function *function = context.getFunction("_ejs_op_typeof_is_array");
    if (function == NULL)
    {
        std::cout <<  __FUNCTION__ << " Can't find _ejs_op_typeof_is_array()" << std::endl;
        context.abort();
        return NULL;
    }
    
    std::vector<llvm::Value *> args;
    args.push_back(jsArrayRef);
	llvm::CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
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
    llvm::Value *isArrayV = CodeGenJSValue32FunctionIsTrue(context, call);

    Function *currentFunction = context.currentFunction();

    // Make the new basic block for the loop header, inserting after current block.
    BasicBlock *asObjectBB = BasicBlock::Create(getGlobalContext(), "forin.asobject", currentFunction);
    BasicBlock *asArrayBB = BasicBlock::Create(getGlobalContext(), "forin.asarray", currentFunction);
    BasicBlock *loopCheckBB = BasicBlock::Create(getGlobalContext(), "forin.check", currentFunction);
    BasicBlock *asArrayLoopBB = BasicBlock::Create(getGlobalContext(), "forin.body.asarray", currentFunction);
    BasicBlock *asObjectLoopBB = BasicBlock::Create(getGlobalContext(), "forin.body.asobject", currentFunction);
    BasicBlock *loopBB = BasicBlock::Create(getGlobalContext(), "forin.body", currentFunction);
    BasicBlock *loopStepBB = BasicBlock::Create(getGlobalContext(), "forin.step");
    BasicBlock *loopEndBB = BasicBlock::Create(getGlobalContext(), "forin.end");

    //     isArray ? branch forin.asarray, forin.asobject
    BranchInst::Create(asArrayBB, asObjectBB, isArrayV, context.currentBlock());

    // forin.asobject
    context.pushBlock(asObjectBB);
    //     arrayRef = _ejs_Object_getOwnPropertyNames(_ejs_undefined, _ejs_undefined, 1, &arrayRef);
    function = context.getFunction("_ejs_Object_getOwnPropertyNames");
    if (function == NULL)
    {
        std::cout <<  __FUNCTION__ << " Can't find _ejs_Object_getOwnPropertyNames()" << std::endl;
        context.abort();
        return NULL;
    }

    //use args above without modify
    llvm::Value *propertiesRef = CodeGenJSObjectFunctionCall(context, function, args);
    CodeGenJSValue32FunctionCopy(context, jsArrayRef, propertiesRef);
    //     branch forin.asarray
    BranchInst::Create(asArrayBB, context.currentBlock());

    // forin.asarray:
    context.pushBlock(asArrayBB);
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    //     jsLength = _ejs_specop_get(arrayRef, _ejs_atom_length, arrayRef);
    llvm::Value *lengthV = CodeGenJSObjectGetPropertyCall(context, jsArrayRef, context.propertyNames->length.name);
    //     length = (int)ToUint32(jsLength);
    lengthV = CodeGenJSValue32FunctionToInt32(context, lengthV);
    llvm::Value *intIndexRef = new llvm::AllocaInst(int32Type, "intIndex", context.currentBlock());
    new llvm::StoreInst(llvm::ConstantInt::get(int32Type, 0), intIndexRef, false, 4, context.currentBlock());
    //     branch forin.check
    BranchInst::Create(loopCheckBB, context.currentBlock());
    
    context.setBreakBlock(loopEndBB);
    context.setContinueBlock(loopStepBB);
    // forin.check
    context.pushBlock(loopCheckBB);
    //     (index<length) ? branch forin.body.asarray, forin.end
    RegisterID *condV = CodeGenJSValue32FunctionCmp(context, intIndexRef, lengthV, op_less);
    BranchInst::Create(asArrayLoopBB, loopEndBB, condV, context.currentBlock());
    context.popBlock(loopCheckBB);
    
    // forin.body.asarray
    context.pushBlock(asArrayLoopBB);
    //     idRef = INT32_TO_EJSVAL_IMPL(index);
    CodeGenJSValue32FunctionCopy(context, idRef, intIndexRef);
    //     isArray ? branch forin.body, forin.body.asobject
    BranchInst::Create(loopBB, asObjectLoopBB, isArrayV, context.currentBlock());

    // forin.body.asobject
    context.pushBlock(asObjectLoopBB);
    //     idRef = _ejs_specop_get(arrayRef, idRef, arrayRef);
    llvm::Value *arrayIndexRef = CodeGenJSObjectGetPropertyCall(context, jsArrayRef, idRef);
    CodeGenJSValue32FunctionCopy(context, idRef, arrayIndexRef);
    //     branch forin.body
    BranchInst::Create(loopBB, context.currentBlock());
    
    // forin.body
    context.pushBlock(loopBB);
    //     m_statement;
    m_statement->emitBytecode(context, dst);
    //     branch forin.step
    if (NULL == context.currentBlock()->getTerminator())
    {
        BranchInst::Create(loopStepBB, context.currentBlock());
    }
    context.popBlock(loopBB);

    // forin.step
    currentFunction->getBasicBlockList().push_back(loopStepBB);
    context.pushBlock(loopStepBB);
    //     index++;
    llvm::Value *indexV = CodeGenJSValue32FunctionOption(context, intIndexRef, llvm::ConstantInt::getTrue(llvm::getGlobalContext()), op_plusplus);
    new llvm::StoreInst(indexV, intIndexRef, false, 4, context.currentBlock());
    //     branch forin.check
    BranchInst::Create(loopCheckBB, context.currentBlock());
    context.popBlock(loopStepBB);
    
    // forin.end
    currentFunction->getBasicBlockList().push_back(loopEndBB);
    context.pushBlock(loopEndBB);
    /**
     * the code blow should follow whileendBB, instead of last block such as "entry"
     * so, do popBlock when function is parsed completly
     */
    //context.popBlock();

    // for expr always returns 0.0.
    return Constant::getNullValue(Type::getDoubleTy(getGlobalContext()));
}

// ------------------------------ JavaScriptCore/ContinueNode ---------------------------------

// ECMA 12.7
RegisterID* ContinueNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    llvm::BasicBlock *block;
    if (m_ident.name.length() <= 0)
    {
        block = context.getContinueBlock();
    }
    else
    {
        block = context.findBlock(m_ident.name);
    }
    if (NULL == block)
    {
        std::cout <<  __FUNCTION__ << " Can't fine block " << m_ident.name << ", Need call CodeGenContext::setContinueBlock(block) before use it !" << std::endl;
        context.abort();
        return NULL;
    }
    return BranchInst::Create(block, context.currentBlock());
}

// ------------------------------ JavaScriptCore/BreakNode ------------------------------------

// ECMA 12.8
RegisterID* BreakNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    llvm::BasicBlock *block;
    if (m_ident.name.length() <= 0)
    {
        block = context.getBreakBlock();
    }
    else
    {
        block = context.findBlock(m_ident.name);
    }
    if (NULL == block)
    {
        std::cout <<  __FUNCTION__ << " Can't fine block " << m_ident.name << ", Need call CodeGenContext::setBreakBlock(block) before use it !" << std::endl;
        context.abort();
        return NULL;
    }
    return BranchInst::Create(block, context.currentBlock());
}

// ------------------------------ JavaScriptCore/ReturnNode -----------------------------------

RegisterID* ReturnNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    RegisterID* jsValueRef = NULL;
    if (NULL == m_value)
    {
        //since 0.0 == 0 == 0x0000000000000000 on 32bit platform, just return 0, directly
#if 0
        jsValueRef = context.getVariableRef("_ejs_zero");
#else
        llvm::Type *int64Type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
        jsValueRef = llvm::ConstantInt::get(int64Type, 0);
#endif
    }
    else
    {
        jsValueRef = m_value->emitBytecode(context, dst);
    }

   RegisterID*  int64Value = CodeGenJSValue32FunctionToInt64(context, jsValueRef);
    
    ReturnInst::Create(getGlobalContext(), int64Value, context.currentBlock());
    return int64Value;
}

// ------------------------------ JavaScriptCore/WithNode -------------------------------------

RegisterID* WithNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

// ------------------------------ JavaScriptCore/CaseClauseNode --------------------------------


// ------------------------------ JavaScriptCore/CaseBlockNode --------------------------------

uint32_t bkdr_hash(const char *str)
{
    uint32_t seed = 131; // the magic number, 31, 131, 1313, 13131, etc.. orz..
    uint32_t hash = 0;
    unsigned char *p = (unsigned char *)str;

    while (*p)
    {
        hash = hash * seed + (*p++);
    }
    
    return hash;
}

uint32_t hash6432shift(uint64_t key)
{
  key = (~key) + (key << 18); // key = (key << 18) - key - 1;
  key = key ^ (key >> 31);
  key = key * 21; // key = (key + (key << 2)) + (key << 4);
  key = key ^ (key >> 11);
  key = key + (key << 6);
  key = key ^ (key >> 22);
  return (uint32_t)key;
}

typedef struct CaseInfo
{
    CaseClauseNode *caseNode;
    SwitchType      type;
    unsigned int    hashCode;
    llvm::BasicBlock *checkBB; //the check block of the case, maybe shared block if have the same hashCode
    llvm::BasicBlock *realBB; //the real block of the case
    struct CaseInfo *next;
}CaseInfo;
static SwitchType initCaseInfo(ClauseListNode *list, std::vector<CaseInfo> &caseInfos)
{
    CaseInfo caseInfo;
    ejsval jsValue;
    BooleanNode *booleanNode;
    NumberNode *numberNode;
    StringNode *stringNode;
    SwitchType switchType = SwitchInteger;

    for (; list; list = list->next())
    {
        ExpressionNode *exprNode = list->getClause()->expr();
        memset((void *)&caseInfo, 0x00, sizeof(CaseInfo));
        caseInfo.caseNode = list->getClause();
        switch (exprNode->opcodeID())
        {
        case op_const_undefined:
            caseInfo.type = SwitchDouble;
            caseInfo.hashCode = hash6432shift(((uint64_t)EJSVAL_TAG_UNDEFINED)<<32 | (uint64_t)0x00000000);
            break;
        case op_const_null:
            caseInfo.type = SwitchDouble;
            caseInfo.hashCode = hash6432shift(((uint64_t)EJSVAL_TAG_NULL)<<32 | (uint64_t)0x00000000);
            break;
        case op_const_boolean:
            booleanNode = (BooleanNode *)exprNode;
            caseInfo.type = SwitchDouble;
            caseInfo.hashCode = hash6432shift(((uint64_t)EJSVAL_TAG_BOOLEAN)<<32 | (uint64_t)booleanNode->value());
            break;
        case op_const_number:
            numberNode = (NumberNode *)exprNode;
            jsValue.asDouble = numberNode->value();
            caseInfo.type = SwitchDouble;
            caseInfo.hashCode = hash6432shift(jsValue.asInt64);
            break;
        case op_const_string:
            stringNode = (StringNode *)exprNode;
            caseInfo.type = SwitchString;
#if defined(JSLANG_UTF8)
            caseInfo.hashCode = bkdr_hash(stringNode->value().name.c_str());
#else
            caseInfo.hashCode = bkdr_hash(stringNode->value().name.c_str());
            #err "Can't deal with charset !"
#endif
            break;
        default:
            caseInfo.type = SwitchUnkown;
            break;
        }
        switchType = (SwitchType)((int)switchType | (int)caseInfo.type);
        caseInfos.push_back(caseInfo);
    }

    return switchType;
}

// ------------------------------ JavaScriptCore/SwitchNode -----------------------------------

RegisterID* SwitchNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    //switch(expr)
    //{
    //caseA:
    //    stmtA
    //caseD:
    //    stmtD
    //default:
    //    stmtdefault
    //casea:
    //    stmta
    //caseb:
    //    stmtb
    //casec:
    //    stmtc
    //
    // note: caseA and casea have the same hashCode
    //
    //Output as:
    //    expr
    //    switch [caseA.checkBB, caseD.checkBB, caseb.checkBB, casec.checkBB]
    //caseA.checkBB:
    //    cond (expr==exprA) & br caseA.realBB, casea.checkBB
    //caseD.checkBB:
    //    cond (expr==exprD) & br caseD.realBB, default.realBB
    //casea.checkBB:
    //    cond (expr==expra) & br casea.realBB, default.realBB
    //caseb.checkBB:
    //    cond (expr==exprb) & br caseb.realBB, default.realBB
    //casec.checkBB:
    //    cond (expr==exprc) & br casec.realBB, default.realBB
    //caseA.realBB:
    //    stmtA
    //caseD.realBB:
    //    stmtD
    //default.realBB:
    //    stmtdefault
    //casea.realBB:
    //    stmta
    //caseb.realBB:
    //    stmtb
    //casec.realBB:
    //    stmtc
    //endBB:
    //    ...
    // so:
    // each case has two block: checkBB and realBB
    // if two case have the same hashCode, the first case.checkBB should branch to the second case.checkBB if failed
    // note:
    // if there isn't a break at the end of stmt, branch to next realBB
    if ((NULL == m_expr) || (NULL == m_block))
    {
        std::cout <<  __FUNCTION__ << " m_expr=" << m_expr << ",m_block=" << m_block << std::endl;
        context.abort();
        return NULL;
    }

    //init all cases in caseInfos, in original order: m_list1, m_defaultClause, m_list2(deal with no break case)
    std::vector<CaseInfo> caseInfos;
    initCaseInfo(m_block->list1(), caseInfos);
    if (NULL != m_block->defaultClause())
    {
        CaseInfo caseInfo;
        memset((void *)&caseInfo, 0x00, sizeof(CaseInfo));
        caseInfo.caseNode = m_block->defaultClause();
        caseInfo.type = SwitchDefault;
        caseInfos.push_back(caseInfo);
    }
    initCaseInfo( m_block->list2(), caseInfos);
    unsigned int hashCode;
    int checkCount = 0;
    int realCount = (int)caseInfos.size();
    llvm::BasicBlock* checkBB;
    llvm::BasicBlock* endBB = BasicBlock::Create(getGlobalContext(), "sw.end");
    llvm::BasicBlock* defaultBB = endBB;

    //merge case via check haseCode,  and create block for each case
    std::map<unsigned int, llvm::BasicBlock*> hashBBs;
    for (int indexX = realCount - 1; indexX >= 0; indexX--)
    {
        if (SwitchDefault == caseInfos[indexX].type)
        {
            // default case has no checkBB
            defaultBB = BasicBlock::Create(getGlobalContext(), "sw.default");
            caseInfos[indexX].realBB = defaultBB;
            continue;
        }
        else if (SwitchUnkown == caseInfos[indexX].type)
        {
            //link all non-const expressions together with next point
            caseInfos[indexX].checkBB = BasicBlock::Create(getGlobalContext(), "sw.nonconst.case");
            caseInfos[indexX].realBB = BasicBlock::Create(getGlobalContext(), "sw.real.case");
            for (int indexY = indexX + 1; indexY < realCount; indexY++)
            {
                if (SwitchUnkown == caseInfos[indexY].type)
                {
                    caseInfos[indexX].next = &caseInfos[indexY];
                    break;
                }
            }
            continue;
        }
        hashCode = caseInfos[indexX].hashCode % realCount;
        checkBB = hashBBs[hashCode];
        if (NULL == checkBB)
        {
            checkCount++;
        }
        else
        {
            for (int indexY = indexX + 1; indexY < realCount; indexY++)
            {
                if (hashCode == caseInfos[indexY].hashCode)
                {
                    caseInfos[indexX].next = &caseInfos[indexY];
                    break;
                }
            }
            if (NULL == caseInfos[indexX].next)
            {
                std::cout <<  __FUNCTION__ << " hash conflict but can't find it !" << std::endl;
                context.abort();
                return NULL;
            }
        }
        checkBB = BasicBlock::Create(getGlobalContext(), "sw.check.case");
        hashBBs[hashCode] = checkBB;
        //store the real hashCode, so we can get hashCode directly atfer the first loop
        caseInfos[indexX].hashCode = hashCode;
        caseInfos[indexX].checkBB = checkBB;
        caseInfos[indexX].realBB = BasicBlock::Create(getGlobalContext(), "sw.real.case");
    }

    Function *currentFunction = context.currentFunction();
    RegisterID *exprRef = m_expr->emitBytecode(context, dst);
    //init and push all non-const checkBBs
    for (int indexX = 0; indexX < realCount; indexX++)
    {
        if (SwitchUnkown != caseInfos[indexX].type)
        {
            continue;
        }

        ExpressionNode *caseExpr = (ExpressionNode *)caseInfos[indexX].caseNode->expr();
        //in non-const check:
        // current non-const expression's checkBB is used for next non-const expression
        // since the first non-const expression use "currentBlock"
        // the last non-const expression's checkBB is used as new "currentBlock"
        llvm::Value *condV = CodeGenJSValue32FunctionCmp(context, exprRef, caseExpr->emitBytecode(context, dst), op_stricteq);
        BranchInst::Create(caseInfos[indexX].realBB, caseInfos[indexX].checkBB, condV, context.currentBlock());
        currentFunction->getBasicBlockList().push_back(caseInfos[indexX].checkBB);
        context.pushBlock(caseInfos[indexX].checkBB);
    }

    //create switch inst at currentBB if needed, if there aren't any non-complex expressions, SwitchInst isn't needed
    // just branch to default case or the end.
    if (checkCount > 0)
    {
        llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
        RegisterID *hashV = CodeGenJSValue32FunctionGetHashCode(context, exprRef);
        BinaryOperator* switchV = BinaryOperator::Create(Instruction::URem, hashV, llvm::ConstantInt::get(int32Type, realCount), "mod", context.currentBlock());
        llvm::SwitchInst *switchInst = llvm::SwitchInst::Create(switchV, defaultBB, realCount, context.currentBlock());
    
        //init and push all const checkBBs
        for (int indexX = 0; indexX < realCount; indexX++)
        {
            if (SwitchDefault == caseInfos[indexX].type)
            {
                continue;
            }
            if (SwitchUnkown == caseInfos[indexX].type)
            {
                continue;
            }
    
            hashCode = caseInfos[indexX].hashCode;
            if (caseInfos[indexX].checkBB == hashBBs[hashCode])
            {
                //the merged entry a same hashCode, add to switchInst
                switchInst->addCase(llvm::ConstantInt::get(int32Type, hashCode), caseInfos[indexX].checkBB);
            }
            
            ExpressionNode *caseExpr = (ExpressionNode *)caseInfos[indexX].caseNode->expr();
            llvm::BasicBlock* falseBB = endBB;
            if (NULL != caseInfos[indexX].next)
            {
                falseBB = caseInfos[indexX].next->checkBB;
            }
            
            currentFunction->getBasicBlockList().push_back(caseInfos[indexX].checkBB);
            context.pushBlock(caseInfos[indexX].checkBB);
            llvm::Value *condV = CodeGenJSValue32FunctionCmp(context, exprRef, caseExpr->emitBytecode(context, dst), op_stricteq);
            BranchInst::Create(caseInfos[indexX].realBB, falseBB, condV, context.currentBlock());
            context.popBlock(caseInfos[indexX].checkBB);
        }
    }
    else
    {
        //if all the cases are non-const cases, no need to create SwitchInst, just branch to defaultBB(endBB if there is no defult) of the statement
        BranchInst::Create(defaultBB, context.currentBlock());
    }

    //init and push all realBBs
    context.setBreakBlock(endBB);
    for (int indexX = 0; indexX < realCount; indexX++)
    {
        SourceElements *caseStmt = (SourceElements *)caseInfos[indexX].caseNode->statments();
        llvm::BasicBlock* continueBB = endBB;
        if (indexX + 1 < realCount)
        {
            continueBB = caseInfos[indexX + 1].realBB;
        }

        currentFunction->getBasicBlockList().push_back(caseInfos[indexX].realBB);
        context.pushBlock(caseInfos[indexX].realBB);
        caseStmt->emitBytecode(context, dst);
        if (NULL == context.currentBlock()->getTerminator())
        {
            BranchInst::Create(continueBB, context.currentBlock());
        }
        context.popBlock(caseInfos[indexX].realBB);
    }

    //push endBB
    currentFunction->getBasicBlockList().push_back(endBB);
    context.pushBlock(endBB);

    return NULL;
}

// ------------------------------ JavaScriptCore/LabelNode ------------------------------------

RegisterID* LabelNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_statement)
    {
        std::cout <<  __FUNCTION__ << " m_statement=NULL" << std::endl;
        context.abort();
        return NULL;
    }
    
    Function *currentFunction = context.currentFunction();
    BasicBlock *lableBB = BasicBlock::Create(getGlobalContext(), m_name.name, currentFunction);

    if (NULL == context.currentBlock()->getTerminator())
    {
        BranchInst::Create(lableBB, context.currentBlock());
    }
    context.pushBlock(lableBB, m_name.name);
    return m_statement->emitBytecode(context, dst);
}

// ------------------------------ JavaScriptCore/ThrowNode ------------------------------------

RegisterID* ThrowNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

// ------------------------------ JavaScriptCore/TryNode --------------------------------------

RegisterID* TryNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

// -----------------------------ScopeNodeData ---------------------------

ScopeNodeData::ScopeNodeData(VarStack* varStack, FunctionStack* funcStack, int numConstants)
    : m_numConstants(numConstants)
    , m_statements(NULL)
{
    if (varStack)
        m_varStack.swap(*varStack);
    if (funcStack)
        m_functionStack.swap(*funcStack);
}

// ------------------------------ JavaScriptCore/ScopeNode -----------------------------

ScopeNode::ScopeNode(CodeGenContext* globalData)
    : StatementNode(globalData)
    , m_data(NULL)
    , m_features(NoFeatures)
{
}

ScopeNode::ScopeNode(CodeGenContext* globalData, SourceElements* children, VarStack* varStack, FunctionStack* funcStack, CodeFeatures features, int numConstants)
    : StatementNode(globalData)
    , m_data(new ScopeNodeData(varStack, funcStack, numConstants))
    , m_features(features)
{
    m_data->m_statements = children;
}

void ScopeNode::emitStatementsBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == m_data)
    {
        return ;
    }
    VarStack::const_iterator it_var;
    for (it_var = m_data->m_varStack.begin(); it_var != m_data->m_varStack.end(); it_var++)
    {
    }
    FunctionStack::const_iterator it_func;
    for (it_func = m_data->m_functionStack.begin(); it_func != m_data->m_functionStack.end(); it_func++)
    {
        (*it_func)->emitBytecode(context, dst);
    }
    if (NULL != m_data->m_statements)
    {
        m_data->m_statements->emitBytecode(context, dst);
    }
}

StatementNode* ScopeNode::singleStatement() const
{
    return m_data->m_statements ? m_data->m_statements->singleStatement() : 0;
}

// ------------------------------ JavaScriptCore/ProgramNode -----------------------------

inline ProgramNode::ProgramNode(CodeGenContext* globalData, SourceElements* children, VarStack* varStack, FunctionStack* funcStack, CodeFeatures features, int numConstants)
    : ScopeNode(globalData, children, varStack, funcStack, features, numConstants)
{
}

ProgramNode* ProgramNode::create(CodeGenContext* globalData, SourceElements* children, VarStack* varStack, FunctionStack* funcStack, CodeFeatures features, int numConstants)
{
    ProgramNode* node = new ProgramNode(globalData, children, varStack, funcStack, features, numConstants);

    ASSERT(node->data()->m_arena.last() == node);
    ASSERT(!node->data()->m_arena.contains(node.get()));

    return node;
}

RegisterID* ProgramNode::emitBytecode(CodeGenContext& context, RegisterID*)
{
    /**
     * ProgramNode is the top node of each *.js file, 
     * I use a global pointer to store it directly at present, and it should be modified in future
     * each *.js file should be parsed in its own Module, and generate a *.obj for link
     * require is a special case, the module which is required will trigger a module searching,
     * if the module isn't included, launch a new *.js file compiling for link
     */
    std::cout <<  __FUNCTION__ << " UNIMPLEMENTED" << std::endl;return NULL;
}

// ------------------------------ JavaScriptCore/EvalNode -----------------------------

inline EvalNode::EvalNode(CodeGenContext* globalData, SourceElements* children, VarStack* varStack, FunctionStack* funcStack, CodeFeatures features, int numConstants)
    : ScopeNode(globalData, children, varStack, funcStack, features, numConstants)
{
}

EvalNode* EvalNode::create(CodeGenContext* globalData, SourceElements* children, VarStack* varStack, FunctionStack* funcStack, CodeFeatures features, int numConstants)
{
    EvalNode* node = new EvalNode(globalData, children, varStack, funcStack, features, numConstants);

    ASSERT(node->data()->m_arena.last() == node);
    ASSERT(!node->data()->m_arena.contains(node.get()));

    return node;
}

RegisterID* EvalNode::emitBytecode(CodeGenContext& context, RegisterID *dst)
{
   //eval is an implicit function declare and function call, so deal with it like FuncDeclNode and FunctionCallResolveNode.
    llvm::IntegerType *returnType = llvm::Type::getInt64Ty(llvm::getGlobalContext());
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    std::vector<Type*> argTypes;
    argTypes.push_back(jsValuePtrType);
    argTypes.push_back(jsValuePtrType);
    argTypes.push_back(int32Type);
    argTypes.push_back(jsValuePtrType);
    FunctionType *ftype = FunctionType::get(returnType, makeArrayRef(argTypes), false);
    Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, "eval.anonymity", context.getModule());
    function->setCallingConv(llvm::CallingConv::C);
    {
        AttributeSet function_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;
        AttrBuilder B1;
        B1.addAttribute(Attribute::ByVal);
        B1.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B1);
        Attrs.push_back(PAS);
        
        PAS = AttributeSet::get(llvm::getGlobalContext(), 2U, B1);
        Attrs.push_back(PAS);
        
        AttrBuilder B2;
        PAS = AttributeSet::get(llvm::getGlobalContext(), ~0U, B2);
        Attrs.push_back(PAS);
        function_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        function->setAttributes(function_PAL);
    }

    BasicBlock *entryBB = BasicBlock::Create(getGlobalContext(), "entry", function, 0);

    Function::arg_iterator argsValues = function->arg_begin();
    llvm::Value* envRef = argsValues++;
    envRef->setName("env.addr");
    llvm::Value* thisRef = argsValues++;
    thisRef->setName("this");
    llvm::Value* argnValue = argsValues++;
    argnValue->setName("argn");
    llvm::Value* argvRef = argsValues++;
    argvRef->setName("argv");

    context.pushBlock(entryBB);
    context.setVariableRef("env.addr", envRef);
    context.setVariableRef("this", thisRef);

    emitStatementsBytecode(context, dst);
    RegisterID *resultRef = Constant::getNullValue(returnType);
    if (NULL == context.currentBlock()->getTerminator())
    {
        ReturnInst::Create(getGlobalContext(), resultRef, context.currentBlock());
    }

    context.popBlock(entryBB);

    std::vector<llvm::Value *> args;
    resultRef = CodeGenJSObjectFunctionCall(context, function, args);

    std::cout <<  __FUNCTION__ << "Creating eval function" << std::endl;

    return resultRef;
}

// ------------------------------ JavaScriptCore/FunctionBodyNode -----------------------------

FunctionParameters::FunctionParameters(ParameterNode* firstParameter)
{
    for (ParameterNode* parameter = firstParameter; parameter; parameter = parameter->next())
        push_back(parameter->ident());
}

inline FunctionBodyNode::FunctionBodyNode(CodeGenContext* globalData, SourceElements* children, VarStack* varStack, FunctionStack* funcStack, CodeFeatures features, int numConstants)
    : ScopeNode(globalData, children, varStack, funcStack, features, numConstants)
{
}

void FunctionBodyNode::finishParsing(ParameterNode* firstParameter, const Identifier& ident)
{
    finishParsing(FunctionParameters::create(firstParameter), ident);
}

void FunctionBodyNode::finishParsing(FunctionParameters* parameters, const Identifier& ident)
{
    ASSERT(!source().isNull());
    m_parameters = parameters;
    m_ident = ident;
}

FunctionBodyNode* FunctionBodyNode::create(CodeGenContext* globalData, SourceElements* children, VarStack* varStack, FunctionStack* funcStack, CodeFeatures features, int numConstants)
{
    FunctionBodyNode* node = new FunctionBodyNode(globalData, children, varStack, funcStack, features, numConstants);

    ASSERT(node->data()->m_arena.last() == node);
    ASSERT(!node->data()->m_arena.contains(node.get()));

    return node;
}

/**
 * every function in js should be JSProc type, excepet the function we generate by clang
 * typedef union ejsval (*JSProc)(union ejsval env, union ejsval thisObject, int argc, union ejsval* argv);
 * clang will optimize the return type from struct to buildin type if needed, so, I have to disable it like this:
 * clang.exe -S -emit-llvm -fpcc-struct-return allinone.c -imacros macros.txt
 * and the real type of JSProc is :
 * void (%union.ejsval*, %union.ejsval*, %union.ejsval*, i32, %union.ejsval*)*
 * the 1st %union.ejsval* is value refrence for return
 * the 2nd %union.ejsval* is value refrence of env
 * the 3rd %union.ejsval* is value refrence of this
 * the 4th i32 is count of real parameters
 * the 5th %union.ejsval* is point to a array of real parameters
 *
 * I have to use default ABI for future case
 */
RegisterID* FunctionBodyNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    llvm::IntegerType *returnType = llvm::Type::getInt64Ty(llvm::getGlobalContext());
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::Type *jsValueType = context.getTypeRef("union.ejsval");
    llvm::Type *jsValuePtrType = llvm::PointerType::get(jsValueType, 0);
    std::vector<Type*> argTypes;
    argTypes.push_back(jsValuePtrType);
    argTypes.push_back(jsValuePtrType);
    argTypes.push_back(int32Type);
    argTypes.push_back(jsValuePtrType);
    FunctionType *ftype = FunctionType::get(returnType, makeArrayRef(argTypes), false);
    Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, m_ident.name.c_str(), context.getModule());
    function->setCallingConv(llvm::CallingConv::C);
    {
        AttributeSet function_PAL;
        SmallVector<AttributeSet, 4> Attrs;
        AttributeSet PAS;
        AttrBuilder B1;
        B1.addAttribute(Attribute::ByVal);
        B1.addAlignmentAttr(4);
        PAS = AttributeSet::get(llvm::getGlobalContext(), 1U, B1);
        Attrs.push_back(PAS);
        
        PAS = AttributeSet::get(llvm::getGlobalContext(), 2U, B1);
        Attrs.push_back(PAS);
        
        AttrBuilder B2;
        PAS = AttributeSet::get(llvm::getGlobalContext(), ~0U, B2);
        Attrs.push_back(PAS);
        function_PAL = AttributeSet::get(llvm::getGlobalContext(), Attrs);
        function->setAttributes(function_PAL);
    }
    
    BasicBlock *entryBB = BasicBlock::Create(getGlobalContext(), "entry", function, 0);
    BasicBlock *ifBB = NULL;
    BasicBlock *thenBB = NULL;
    BasicBlock *realentryBB = entryBB;

    Function::arg_iterator argsValues = function->arg_begin();
    llvm::Value* envRef = argsValues++;
    envRef->setName("env.addr");
    llvm::Value* thisRef = argsValues++;
    thisRef->setName("this");
    llvm::Value* argnValue = argsValues++;
    argnValue->setName("argn");
    llvm::Value* argvRef = argsValues++;
    argvRef->setName("argv");

    std::vector<Identifier>::const_iterator initIterator;
    int index = 0;
    llvm::Value* parameterRef;
    for (initIterator = parameters()->begin(); initIterator != parameters()->end(); initIterator++,index++)
    {
        if (NULL == thenBB)
        {
            ifBB = BasicBlock::Create(getGlobalContext(), "init", function, entryBB);
            realentryBB = ifBB;
            context.pushBlock(ifBB);
            for (std::vector<Identifier>::const_iterator allocIterator = parameters()->begin(); allocIterator != parameters()->end(); allocIterator++)
            {
                parameterRef = CodeGenJSValue32ConstructForUndefined(context, (*allocIterator).name.c_str());
                context.setVariableRef((*allocIterator).name, parameterRef);
            }
            //Should not pop it
            //context.popBlock(ifBB);
        }
        else
        {
            ifBB = thenBB;
            context.pushBlock(ifBB);
        }
        thenBB = BasicBlock::Create(getGlobalContext(), "init", function, entryBB);

        // if (argn > index)
        // compare to check whether has the paramter or not
        llvm::Value* indexValue = ConstantInt::get(int32Type, index);
        ICmpInst* condV = new ICmpInst(*ifBB, ICmpInst::ICMP_SGT, argnValue, indexValue, "cmp");
        BranchInst::Create(thenBB, entryBB, condV, ifBB);

        //init parameter
        GetElementPtrInst* argvIndex = GetElementPtrInst::Create(argvRef, indexValue, "argvIndex", thenBB);
        //new StoreInst(argvIndex, parameterRef, false, thenBB);
        context.pushBlock(thenBB);
        parameterRef = context.getVariableRef((*initIterator).name);
        CodeGenJSValue32FunctionCopy(context, parameterRef, argvIndex);
        context.popBlock(thenBB);
    }

    if (NULL != thenBB)
    {
        BranchInst::Create(entryBB, thenBB);
    }
    
    context.pushBlock(entryBB);
    context.setVariableRef("env.addr", envRef);
    context.setVariableRef("this", thisRef);
    if (1)
    {
        //It is really a bad idea to create arguments for every function
        //should save a (Instruction *InsertBefore) pointer for each function
        //if we want to declare some variables, just add instructions before it
        Function *function_arguments = context.getFunction("_ejs_arguments_new");
        if (function_arguments == NULL)
        {
            std::cout <<  __FUNCTION__ << " Can't find _ejs_arguments_new()" << std::endl;
            context.abort();
            return NULL;
        }
        
        std::vector<RegisterID*> args;
        args.push_back(argnValue);
        args.push_back(argvRef);
        llvm::Value* arguments = CallInst::Create(function_arguments, makeArrayRef(args), "", context.currentBlock());
        llvm::Value* argumentRef = CodeGenJSValue32Construct(context, "arguments");
        CodeGenJSValue32FunctionCopy(context, argumentRef, arguments);
        context.setVariableRef("arguments", argumentRef);
    }
    else
    {
        context.setVariableRef("arguments", argvRef);
    }

    //the function maybe closure, so, inhert variables from parent/predecessor first
    //but how to deal with this:
    // function foo(n) {
    //   var v = function(vn) {
    //        if (vn < 2)
    //            return 1;
    //        return vn + v(vn-1);
    //   }
    // }
    // in the anonymous closure function, the parent has't init v yet,
    context.cloneVariable();

    emitStatementsBytecode(context, dst);

    if (NULL == context.currentBlock()->getTerminator())
    {
        ReturnInst::Create(getGlobalContext(), Constant::getNullValue(returnType), context.currentBlock());
    }

    context.popBlock(realentryBB);

    context.setVariableRef(m_ident.name, function, false);
    std::cout <<  __FUNCTION__ << "Creating function: " << m_ident.name.c_str() << std::endl;

    return function;
}

// ------------------------------ JavaScriptCore/FuncDeclNode ---------------------------------

RegisterID* FuncDeclNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    if (NULL == body())
    {
        return NULL;
    }
    
    return body()->emitBytecode(context, dst);
}

// ------------------------------ JavaScriptCore/FuncExprNode ---------------------------------

RegisterID* FuncExprNode::emitBytecode(CodeGenContext& context, RegisterID* dst)
{
    //var v = function() { }
    if (NULL == body())
    {
        return NULL;
    }
    
    return body()->emitBytecode(context, dst);
    //update the variables of parent here
}

} // namespace JSC

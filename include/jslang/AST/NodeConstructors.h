/*
 *  Copyright (C) 2009 Apple Inc. All rights reserved.
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

#ifndef NodeConstructors_h
#define NodeConstructors_h

#include "Nodes.h"

namespace JSC {

    inline Node::Node(CodeGenContext* globalData, OpcodeID opcodeID)
        : m_opcodeID(opcodeID)
        , m_line(0)
    {
    }

    inline ExpressionNode::ExpressionNode(CodeGenContext* globalData, OpcodeID opcodeID, ResultType resultType)
        : Node(globalData, opcodeID)
        , m_resultType(resultType)
    {
    }

    inline StatementNode::StatementNode(CodeGenContext* globalData, OpcodeID opcodeID)
        : Node(globalData, opcodeID)
        , m_lastLine(-1)
    {
    }

    inline NullNode::NullNode(CodeGenContext* globalData)
        : ExpressionNode(globalData, op_const_null, ResultType::nullType())
    {
    }

    inline BooleanNode::BooleanNode(CodeGenContext* globalData, bool value)
        : ExpressionNode(globalData, op_const_boolean, ResultType::booleanType())
        , m_value(value)
    {
    }

    inline NumberNode::NumberNode(CodeGenContext* globalData, double value)
        : ExpressionNode(globalData, op_const_number, ResultType::numberType())
        , m_value(value)
    {
    }

    inline StringNode::StringNode(CodeGenContext* globalData, const Identifier& value)
        : ExpressionNode(globalData, op_const_string, ResultType::stringType())
        , m_value(value)
    {
    }

    inline RegExpNode::RegExpNode(CodeGenContext* globalData, const Identifier& pattern, const Identifier& flags)
        : ExpressionNode(globalData)
        , m_pattern(pattern)
        , m_flags(flags)
    {
    }

    inline ThisNode::ThisNode(CodeGenContext* globalData)
        : ExpressionNode(globalData)
    {
    }

    inline ResolveNode::ResolveNode(CodeGenContext* globalData, const Identifier& ident, int startOffset)
        : ExpressionNode(globalData, op_resolve)
        , m_ident(ident)
        , m_startOffset(startOffset)
    {
    }

    inline ElementNode::ElementNode(CodeGenContext*, int elision, ExpressionNode* node, ElementNode* last)
        : m_elision(elision)
        , m_node(node)
    {
        if (NULL != last)
        {
            last->setNext(this);
        }
    }

    inline ArrayNode::ArrayNode(CodeGenContext* globalData, int elision, ElementNode* element)
        : ExpressionNode(globalData)
        , m_element(element)
        , m_elision(elision)
        , m_optional(true)
    {
        if (NULL == element)
        {
            m_optional = true;
        }
        else if (0 != elision)
        {
        }
    }

    inline PropertyNode::PropertyNode(CodeGenContext *globalData, const Identifier& ident, ExpressionNode* assign, Type type)
        : m_ident(ident)
        , m_number(0)
        , m_assign(assign)
        , m_type(type)
    {
    }

    inline PropertyNode::PropertyNode(CodeGenContext *globalData, double number, ExpressionNode* assign, Type type)
        : m_ident()
        , m_number(number)
        , m_assign(assign)
        , m_type(type)
    {
    }

    inline PropertyListNode::PropertyListNode(CodeGenContext* globalData, PropertyNode* node, PropertyListNode* last)
        : Node(globalData, op_end)
        , m_node(node)
    {
        if (NULL != last)
        {
            last->setNext(this);
        }
    }

    inline ObjectLiteralNode::ObjectLiteralNode(CodeGenContext* globalData, PropertyListNode* list)
        : ExpressionNode(globalData)
        , m_list(list)
    {
    }

    inline BracketAccessorNode::BracketAccessorNode(CodeGenContext* globalData, ExpressionNode* base, ExpressionNode* subscript, bool subscriptHasAssignments)
        : ExpressionNode(globalData)
        , m_base(base)
        , m_subscript(subscript)
        , m_subscriptHasAssignments(subscriptHasAssignments)
    {
    }

    inline DotAccessorNode::DotAccessorNode(CodeGenContext* globalData, ExpressionNode* base, const Identifier& ident)
        : ExpressionNode(globalData)
        , m_base(base)
        , m_ident(ident)
    {
    }

    inline ArgumentListNode::ArgumentListNode(CodeGenContext* globalData, ExpressionNode* expr, ArgumentListNode* last)
        : Node(globalData, op_end)
        , m_expr(expr)
    {
        if (NULL != last)
        {
            last->setNext(this);
        }
    }

    inline ArgumentsNode::ArgumentsNode(CodeGenContext*)
        : m_listNode(0)
    {
    }

    inline ArgumentsNode::ArgumentsNode(CodeGenContext*, ArgumentListNode* listNode)
        : m_listNode(listNode)
    {
    }

    inline NewExprNode::NewExprNode(CodeGenContext* globalData, ExpressionNode* expr)
        : ExpressionNode(globalData)
        , m_expr(expr)
        , m_args(0)
    {
    }

    inline NewExprNode::NewExprNode(CodeGenContext* globalData, ExpressionNode* expr, ArgumentsNode* args)
        : ExpressionNode(globalData)
        , m_expr(expr)
        , m_args(args)
    {
    }

    inline EvalFunctionCallNode::EvalFunctionCallNode(CodeGenContext* globalData, ArgumentsNode* args, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData)
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_args(args)
    {
    }

    inline FunctionCallValueNode::FunctionCallValueNode(CodeGenContext* globalData, ExpressionNode* expr, ArgumentsNode* args, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData)
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_expr(expr)
        , m_args(args)
    {
    }

    inline FunctionCallResolveNode::FunctionCallResolveNode(CodeGenContext* globalData, const Identifier& ident, ArgumentsNode* args, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData)
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_ident(ident)
        , m_args(args)
    {
    }

    inline FunctionCallBracketNode::FunctionCallBracketNode(CodeGenContext* globalData, ExpressionNode* base, ExpressionNode* subscript, ArgumentsNode* args, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData)
        , ThrowableSubExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_subscript(subscript)
        , m_args(args)
    {
    }

    inline FunctionCallDotNode::FunctionCallDotNode(CodeGenContext* globalData, ExpressionNode* base, const Identifier& ident, ArgumentsNode* args, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData)
        , ThrowableSubExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_ident(ident)
        , m_args(args)
    {
    }

    inline CallFunctionCallDotNode::CallFunctionCallDotNode(CodeGenContext* globalData, ExpressionNode* base, const Identifier& ident, ArgumentsNode* args, unsigned divot, unsigned startOffset, unsigned endOffset)
        : FunctionCallDotNode(globalData, base, ident, args, divot, startOffset, endOffset)
    {
    }

    inline ApplyFunctionCallDotNode::ApplyFunctionCallDotNode(CodeGenContext* globalData, ExpressionNode* base, const Identifier& ident, ArgumentsNode* args, unsigned divot, unsigned startOffset, unsigned endOffset)
        : FunctionCallDotNode(globalData, base, ident, args, divot, startOffset, endOffset)
    {
    }

    inline PrePostResolveNode::PrePostResolveNode(CodeGenContext* globalData, OpcodeID opcodeID, const Identifier& ident, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData, opcodeID, ResultType::numberType()) // could be reusable for pre?
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_ident(ident)
    {
    }

    inline PostfixResolveNode::PostfixResolveNode(CodeGenContext* globalData, const Identifier& ident, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset)
        : PrePostResolveNode(globalData, opcodeID, ident, divot, startOffset, endOffset)
    {
    }

    inline PostfixBracketNode::PostfixBracketNode(CodeGenContext* globalData, ExpressionNode* base, ExpressionNode* subscript, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData, opcodeID)
        , ThrowableSubExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_subscript(subscript)
    {
    }

    inline PostfixDotNode::PostfixDotNode(CodeGenContext* globalData, ExpressionNode* base, const Identifier& ident, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData, opcodeID)
        , ThrowableSubExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_ident(ident)
    {
    }

    inline PostfixErrorNode::PostfixErrorNode(CodeGenContext* globalData, ExpressionNode* expr, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData, opcodeID)
        , ThrowableSubExpressionData(divot, startOffset, endOffset)
        , m_expr(expr)
    {
    }

    inline DeleteResolveNode::DeleteResolveNode(CodeGenContext* globalData, const Identifier& ident, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData)
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_ident(ident)
    {
    }

    inline DeleteBracketNode::DeleteBracketNode(CodeGenContext* globalData, ExpressionNode* base, ExpressionNode* subscript, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData)
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_subscript(subscript)
    {
    }

    inline DeleteDotNode::DeleteDotNode(CodeGenContext* globalData, ExpressionNode* base, const Identifier& ident, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData)
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_ident(ident)
    {
    }

    inline DeleteValueNode::DeleteValueNode(CodeGenContext* globalData, ExpressionNode* expr)
        : ExpressionNode(globalData)
        , m_expr(expr)
    {
    }

    inline VoidNode::VoidNode(CodeGenContext* globalData, ExpressionNode* expr)
        : ExpressionNode(globalData)
        , m_expr(expr)
    {
    }

    inline TypeOfResolveNode::TypeOfResolveNode(CodeGenContext* globalData, const Identifier& ident)
        : ExpressionNode(globalData, op_typeof, ResultType::stringType())
        , m_ident(ident)
    {
    }

    inline TypeOfValueNode::TypeOfValueNode(CodeGenContext* globalData, ExpressionNode* expr)
        : ExpressionNode(globalData, op_typeof, ResultType::stringType())
        , m_expr(expr)
    {
    }

    inline PrefixResolveNode::PrefixResolveNode(CodeGenContext* globalData, const Identifier& ident, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset)
        : PrePostResolveNode(globalData, opcodeID, ident, divot, startOffset, endOffset)
    {
    }

    inline PrefixBracketNode::PrefixBracketNode(CodeGenContext* globalData, ExpressionNode* base, ExpressionNode* subscript, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData, opcodeID)
        , ThrowablePrefixedSubExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_subscript(subscript)
    {
    }

    inline PrefixDotNode::PrefixDotNode(CodeGenContext* globalData, ExpressionNode* base, const Identifier& ident, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData, opcodeID)
        , ThrowablePrefixedSubExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_ident(ident)
    {
    }

    inline PrefixErrorNode::PrefixErrorNode(CodeGenContext* globalData, ExpressionNode* expr, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData, opcodeID)
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_expr(expr)
    {
    }

    inline UnaryOpNode::UnaryOpNode(CodeGenContext* globalData, ResultType type, ExpressionNode* expr, OpcodeID opcodeID)
        : ExpressionNode(globalData, opcodeID, type)
        , m_expr(expr)
    {
    }

    inline UnaryPlusNode::UnaryPlusNode(CodeGenContext* globalData, ExpressionNode* expr)
        : UnaryOpNode(globalData, ResultType::numberType(), expr, op_to_number)
    {
    }

    inline NegateNode::NegateNode(CodeGenContext* globalData, ExpressionNode* expr)
        : UnaryOpNode(globalData, ResultType::numberTypeCanReuse(), expr, op_negate)
    {
    }

    inline BitwiseNotNode::BitwiseNotNode(CodeGenContext* globalData, ExpressionNode* expr)
        : UnaryOpNode(globalData, ResultType::forBitOp(), expr, op_bitnot)
    {
    }

    inline LogicalNotNode::LogicalNotNode(CodeGenContext* globalData, ExpressionNode* expr)
        : UnaryOpNode(globalData, ResultType::booleanType(), expr, op_not)
    {
    }

    inline BinaryOpNode::BinaryOpNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID, bool rightHasAssignments)
        : ExpressionNode(globalData, opcodeID)
        , m_expr1(expr1)
        , m_expr2(expr2)
        , m_rightHasAssignments(rightHasAssignments)
    {
    }

    inline BinaryOpNode::BinaryOpNode(CodeGenContext* globalData, ResultType type, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID, bool rightHasAssignments)
        : ExpressionNode(globalData, opcodeID, type)
        , m_expr1(expr1)
        , m_expr2(expr2)
        , m_rightHasAssignments(rightHasAssignments)
    {
    }

    inline ReverseBinaryOpNode::ReverseBinaryOpNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID, bool rightHasAssignments)
        : BinaryOpNode(globalData, expr1, expr2, opcodeID, rightHasAssignments)
    {
    }

    inline ReverseBinaryOpNode::ReverseBinaryOpNode(CodeGenContext* globalData, ResultType type, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID, bool rightHasAssignments)
        : BinaryOpNode(globalData, type, expr1, expr2, opcodeID, rightHasAssignments)
    {
    }

    inline MultNode::MultNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::numberTypeCanReuse(), expr1, expr2, op_mul, rightHasAssignments)
    {
    }

    inline DivNode::DivNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::numberTypeCanReuse(), expr1, expr2, op_div, rightHasAssignments)
    {
    }


    inline ModNode::ModNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::numberTypeCanReuse(), expr1, expr2, op_mod, rightHasAssignments)
    {
    }

    inline AddNode::AddNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::forAdd(expr1->resultDescriptor(), expr2->resultDescriptor()), expr1, expr2, op_add, rightHasAssignments)
    {
    }

    inline SubNode::SubNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::numberTypeCanReuse(), expr1, expr2, op_sub, rightHasAssignments)
    {
    }

    inline LeftShiftNode::LeftShiftNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::forBitOp(), expr1, expr2, op_lshift, rightHasAssignments)
    {
    }

    inline RightShiftNode::RightShiftNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::forBitOp(), expr1, expr2, op_rshift, rightHasAssignments)
    {
    }

    inline UnsignedRightShiftNode::UnsignedRightShiftNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::numberTypeCanReuse(), expr1, expr2, op_urshift, rightHasAssignments)
    {
    }

    inline LessNode::LessNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::booleanType(), expr1, expr2, op_less, rightHasAssignments)
    {
    }

    inline GreaterNode::GreaterNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : ReverseBinaryOpNode(globalData, ResultType::booleanType(), expr1, expr2, op_greater, rightHasAssignments)
    {
    }

    inline LessEqNode::LessEqNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::booleanType(), expr1, expr2, op_lesseq, rightHasAssignments)
    {
    }

    inline GreaterEqNode::GreaterEqNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : ReverseBinaryOpNode(globalData, ResultType::booleanType(), expr1, expr2, op_greatereq, rightHasAssignments)
    {
    }

    inline ThrowableBinaryOpNode::ThrowableBinaryOpNode(CodeGenContext* globalData, ResultType type, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID, bool rightHasAssignments)
        : BinaryOpNode(globalData, type, expr1, expr2, opcodeID, rightHasAssignments)
    {
    }

    inline ThrowableBinaryOpNode::ThrowableBinaryOpNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID, bool rightHasAssignments)
        : BinaryOpNode(globalData, expr1, expr2, opcodeID, rightHasAssignments)
    {
    }

    inline InstanceOfNode::InstanceOfNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : ThrowableBinaryOpNode(globalData, ResultType::booleanType(), expr1, expr2, op_instanceof, rightHasAssignments)
    {
    }

    inline InNode::InNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : ThrowableBinaryOpNode(globalData, expr1, expr2, op_in, rightHasAssignments)
    {
    }

    inline EqualNode::EqualNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::booleanType(), expr1, expr2, op_eq, rightHasAssignments)
    {
    }

    inline NotEqualNode::NotEqualNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::booleanType(), expr1, expr2, op_neq, rightHasAssignments)
    {
    }

    inline StrictEqualNode::StrictEqualNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::booleanType(), expr1, expr2, op_stricteq, rightHasAssignments)
    {
    }

    inline NotStrictEqualNode::NotStrictEqualNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::booleanType(), expr1, expr2, op_nstricteq, rightHasAssignments)
    {
    }

    inline BitAndNode::BitAndNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::forBitOp(), expr1, expr2, op_bitand, rightHasAssignments)
    {
    }

    inline BitOrNode::BitOrNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::forBitOp(), expr1, expr2, op_bitor, rightHasAssignments)
    {
    }

    inline BitXOrNode::BitXOrNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
        : BinaryOpNode(globalData, ResultType::forBitOp(), expr1, expr2, op_bitxor, rightHasAssignments)
    {
    }

    inline LogicalOpNode::LogicalOpNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID)
        : BinaryOpNode(globalData, ResultType::booleanType(), expr1, expr2, opcodeID, false)
    {
    }

    inline ConditionalNode::ConditionalNode(CodeGenContext* globalData, ExpressionNode* logical, ExpressionNode* expr1, ExpressionNode* expr2)
        : ExpressionNode(globalData)
        , m_logical(logical)
        , m_expr1(expr1)
        , m_expr2(expr2)
    {
    }

    inline ReadModifyResolveNode::ReadModifyResolveNode(CodeGenContext* globalData, const Identifier& ident, OpcodeID opcodeID, ExpressionNode*  right, bool rightHasAssignments, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData, opcodeID)
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_ident(ident)
        , m_right(right)
        , m_rightHasAssignments(rightHasAssignments)
    {
    }

    inline AssignResolveNode::AssignResolveNode(CodeGenContext* globalData, const Identifier& ident, ExpressionNode* right, bool rightHasAssignments)
        : ResolveNode(globalData, ident, 0)
        , m_right(right)
        , m_rightHasAssignments(rightHasAssignments)
    {
    }

    inline ReadModifyBracketNode::ReadModifyBracketNode(CodeGenContext* globalData, ExpressionNode* base, ExpressionNode* subscript, OpcodeID opcodeID, ExpressionNode* right, bool subscriptHasAssignments, bool rightHasAssignments, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData, opcodeID)
        , ThrowableSubExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_subscript(subscript)
        , m_right(right)
        , m_subscriptHasAssignments(subscriptHasAssignments)
        , m_rightHasAssignments(rightHasAssignments)
    {
    }

    inline AssignBracketNode::AssignBracketNode(CodeGenContext* globalData, ExpressionNode* base, ExpressionNode* subscript, ExpressionNode* right, bool subscriptHasAssignments, bool rightHasAssignments, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData)
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_subscript(subscript)
        , m_right(right)
        , m_subscriptHasAssignments(subscriptHasAssignments)
        , m_rightHasAssignments(rightHasAssignments)
    {
    }

    inline AssignDotNode::AssignDotNode(CodeGenContext* globalData, ExpressionNode* base, const Identifier& ident, ExpressionNode* right, bool rightHasAssignments, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData)
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_ident(ident)
        , m_right(right)
        , m_rightHasAssignments(rightHasAssignments)
    {
    }

    inline ReadModifyDotNode::ReadModifyDotNode(CodeGenContext* globalData, ExpressionNode* base, const Identifier& ident, OpcodeID opcodeID, ExpressionNode* right, bool rightHasAssignments, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData, opcodeID)
        , ThrowableSubExpressionData(divot, startOffset, endOffset)
        , m_base(base)
        , m_ident(ident)
        , m_right(right)
        , m_rightHasAssignments(rightHasAssignments)
    {
    }

    inline AssignErrorNode::AssignErrorNode(CodeGenContext* globalData, ExpressionNode* left, OpcodeID opcodeID, ExpressionNode* right, unsigned divot, unsigned startOffset, unsigned endOffset)
        : ExpressionNode(globalData, opcodeID)
        , ThrowableExpressionData(divot, startOffset, endOffset)
        , m_left(left)
        , m_right(right)
    {
    }

    inline CommaNode::CommaNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2)
        : ExpressionNode(globalData, op_resolve_list)
    {
        m_expressions.push_back(expr1);
        m_expressions.push_back(expr2);
    }

    inline ConstStatementNode::ConstStatementNode(CodeGenContext* globalData, ConstDeclNode* list)
        : StatementNode(globalData)
        , m_list(list)
    {
    }

    inline SourceElements::SourceElements(CodeGenContext* globalData)
        : Node(globalData, op_end)
    {
    }

    inline EmptyStatementNode::EmptyStatementNode(CodeGenContext* globalData)
        : StatementNode(globalData)
    {
    }

    inline DebuggerStatementNode::DebuggerStatementNode(CodeGenContext* globalData)
        : StatementNode(globalData)
    {
    }
    
    inline ExprStatementNode::ExprStatementNode(CodeGenContext* globalData, ExpressionNode* expr)
        : StatementNode(globalData)
        , m_expr(expr)
    {
    }

    inline VarStatementNode::VarStatementNode(CodeGenContext* globalData, ExpressionNode* expr)
        : StatementNode(globalData)
        , m_expr(expr)
    {
    }
    
    inline IfNode::IfNode(CodeGenContext* globalData, ExpressionNode* condition, StatementNode* ifBlock)
        : StatementNode(globalData)
        , m_condition(condition)
        , m_ifBlock(ifBlock)
    {
    }

    inline IfElseNode::IfElseNode(CodeGenContext* globalData, ExpressionNode* condition, StatementNode* ifBlock, StatementNode* elseBlock)
        : IfNode(globalData, condition, ifBlock)
        , m_elseBlock(elseBlock)
    {
    }

    inline DoWhileNode::DoWhileNode(CodeGenContext* globalData, StatementNode* statement, ExpressionNode* expr)
        : StatementNode(globalData)
        , m_statement(statement)
        , m_expr(expr)
    {
    }

    inline WhileNode::WhileNode(CodeGenContext* globalData, ExpressionNode* expr, StatementNode* statement)
        : StatementNode(globalData)
        , m_expr(expr)
        , m_statement(statement)
    {
    }

    inline ForNode::ForNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, ExpressionNode* expr3, StatementNode* statement, bool expr1WasVarDecl)
        : StatementNode(globalData)
        , m_expr1(expr1)
        , m_expr2(expr2)
        , m_expr3(expr3)
        , m_statement(statement)
        , m_expr1WasVarDecl(expr1 && expr1WasVarDecl)
    {
        ASSERT(statement);
    }

    inline ContinueNode::ContinueNode(CodeGenContext* globalData)
        : StatementNode(globalData)
        , m_ident(globalData->propertyNames->nullIdentifier)
    {
    }

    inline ContinueNode::ContinueNode(CodeGenContext* globalData, const Identifier& ident)
        : StatementNode(globalData)
        , m_ident(ident)
    {
    }
    
    inline BreakNode::BreakNode(CodeGenContext* globalData)
        : StatementNode(globalData)
        , m_ident(globalData->propertyNames->nullIdentifier)
    {
    }

    inline BreakNode::BreakNode(CodeGenContext* globalData, const Identifier& ident)
        : StatementNode(globalData)
        , m_ident(ident)
    {
    }
    
    inline ReturnNode::ReturnNode(CodeGenContext* globalData, ExpressionNode* value)
        : StatementNode(globalData, op_ret)
        , m_value(value)
    {
    }

    inline WithNode::WithNode(CodeGenContext* globalData, ExpressionNode* expr, StatementNode* statement, uint32_t divot, uint32_t expressionLength)
        : StatementNode(globalData)
        , m_expr(expr)
        , m_statement(statement)
        , m_divot(divot)
        , m_expressionLength(expressionLength)
    {
    }

    inline LabelNode::LabelNode(CodeGenContext* globalData, const Identifier& name, StatementNode* statement)
        : StatementNode(globalData)
        , m_name(name)
        , m_statement(statement)
    {
    }

    inline ThrowNode::ThrowNode(CodeGenContext* globalData, ExpressionNode* expr)
        : StatementNode(globalData, op_throw)
        , m_expr(expr)
    {
    }

    inline TryNode::TryNode(CodeGenContext* globalData, StatementNode* tryBlock, const Identifier& exceptionIdent, bool catchHasEval, StatementNode* catchBlock, StatementNode* finallyBlock)
        : StatementNode(globalData)
        , m_tryBlock(tryBlock)
        , m_exceptionIdent(exceptionIdent)
        , m_catchBlock(catchBlock)
        , m_finallyBlock(finallyBlock)
        , m_catchHasEval(catchHasEval)
    {
    }

    inline ParameterNode::ParameterNode(CodeGenContext*, const Identifier& ident, ParameterNode* last)
        : m_ident(ident)
    {
        if (NULL != last)
        {
            last->setNext(this);
        }
    }

    inline FuncExprNode::FuncExprNode(CodeGenContext* globalData, const Identifier& ident, FunctionBodyNode* body, ParameterNode* parameter)
        : ExpressionNode(globalData)
        , m_body(body)
    {
        m_body->finishParsing(parameter, ident);
    }

    inline FuncDeclNode::FuncDeclNode(CodeGenContext* globalData, const Identifier& ident, FunctionBodyNode* body, ParameterNode* parameter)
        : StatementNode(globalData)
        , m_body(body)
    {
        m_body->finishParsing(parameter, ident);
    }

    inline CaseClauseNode::CaseClauseNode(CodeGenContext*, ExpressionNode* expr, SourceElements* statements)
        : m_expr(expr)
        , m_statements(statements)
    {
    }

    inline ClauseListNode::ClauseListNode(CodeGenContext*, CaseClauseNode* clause, ClauseListNode* last)
        : m_clause(clause)
    {
        if (NULL != last)
        {
            last->setNext(this);
        }
    }

    inline CaseBlockNode::CaseBlockNode(CodeGenContext*, ClauseListNode* list1, CaseClauseNode* defaultClause, ClauseListNode* list2)
        : m_list1(list1)
        , m_defaultClause(defaultClause)
        , m_list2(list2)
    {
    }

    inline SwitchNode::SwitchNode(CodeGenContext* globalData, ExpressionNode* expr, CaseBlockNode* block)
        : StatementNode(globalData)
        , m_expr(expr)
        , m_block(block)
    {
    }

    inline ConstDeclNode::ConstDeclNode(CodeGenContext* globalData, const Identifier& ident, ExpressionNode* init)
        : ExpressionNode(globalData)
        , m_ident(ident)
        , m_init(init)
    {
    }

    inline BlockNode::BlockNode(CodeGenContext* globalData, SourceElements* statements)
        : StatementNode(globalData)
        , m_statements(statements)
    {
    }

    inline ForInNode::ForInNode(CodeGenContext* globalData, ExpressionNode* l, ExpressionNode* expr, StatementNode* statement)
        : StatementNode(globalData)
        , m_ident(globalData->propertyNames->nullIdentifier)
        , m_init(0)
        , m_lexpr(l)
        , m_expr(expr)
        , m_statement(statement)
        , m_identIsVarDecl(false)
    {
    }

    inline ForInNode::ForInNode(CodeGenContext* globalData, const Identifier& ident, ExpressionNode* in, ExpressionNode* expr, StatementNode* statement, int divot, int startOffset, int endOffset)
        : StatementNode(globalData)
        , m_ident(ident)
        , m_init(0)
        , m_lexpr(new ResolveNode(globalData, ident, divot - startOffset))
        , m_expr(expr)
        , m_statement(statement)
        , m_identIsVarDecl(true)
    {
        if (in) {
            AssignResolveNode* node = new AssignResolveNode(globalData, ident, in, true);
            node->setExceptionSourceCode(divot, divot - startOffset, endOffset - divot);
            m_init = node;
        }
        // for( var foo = bar in baz )
    }

} // namespace JSC

#endif // NodeConstructors_h

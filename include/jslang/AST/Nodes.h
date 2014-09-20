/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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

#ifndef Nodes_h
#define Nodes_h

#include "TypePortingForJSC.h"
#include "ResultType.h"
#if JSLANG_VER == 2
#include "JSTypeTree.h"
#endif
#include <vector>

namespace JSC {

    class ArgumentListNode;
    class FunctionBodyNode;
    class PropertyListNode;
    class ReadModifyResolveNode;
    class Identifier;

    typedef unsigned CodeFeatures;

    const CodeFeatures NoFeatures = 0;
    const CodeFeatures EvalFeature = 1 << 0;
    const CodeFeatures ClosureFeature = 1 << 1;
    const CodeFeatures AssignFeature = 1 << 2;
    const CodeFeatures ArgumentsFeature = 1 << 3;
    const CodeFeatures WithFeature = 1 << 4;
    const CodeFeatures CatchFeature = 1 << 5;
    const CodeFeatures ThisFeature = 1 << 6;
    const CodeFeatures AllFeatures = EvalFeature | ClosureFeature | AssignFeature | ArgumentsFeature | WithFeature | CatchFeature | ThisFeature;

    namespace DeclarationStacks {
        enum VarAttrs { IsConstant = 1, HasInitializer = 2 };
        typedef std::vector<std::pair<const Identifier*, unsigned> > VarStack;
        typedef std::vector<FunctionBodyNode*> FunctionStack;
    }

    enum SwitchType 
    {
        SwitchInteger = 0x00,
        SwitchDouble = 0x01,
        SwitchString = 0x02,
        SwitchUnkown = 0x10,
        SwitchDefault = 0x11,
    };

    class Identifier {
    public:
    	std::string name;
    	Identifier(const std::string& id) : name(id) { }
    	Identifier(const char *id = "") : name(id) { }
    	Identifier(const char *id, int length) : name(id, length) { }
        friend bool operator==(const Identifier &ident, const Identifier&);
        friend bool operator!=(const Identifier &ident, const Identifier&);

        friend bool operator==(const Identifier &ident, const char*);
        friend bool operator!=(const Identifier &ident, const char*);
    };
    
    enum ErrorType {
        GeneralError   = 0,
        EvalError      = 1,
        RangeError     = 2,
        ReferenceError = 3,
        SyntaxError    = 4,
        TypeError      = 5,
        URIError       = 6
    };

    template <typename T>
    class NodeList {
    public:
        NodeList() : m_next(0)   { }

    public:
        T *next() { return m_next; }
        void setNext(T *next) {   m_next = next; }

    private:
        T *m_next;
    };

    class ParserArenaFreeable {
    public:
        // ParserArenaFreeable objects are are freed when the arena is deleted.
        // Destructors are not called. Clients must not call delete on such objects.
        //void* operator new(size_t, CodeGenContext*);
    };

    class ParserArenaDeletable {
    public:
        virtual ~ParserArenaDeletable() { }

        // ParserArenaDeletable objects are deleted when the arena is deleted.
        // Clients must not call delete directly on such objects.
        //void* operator new(size_t, CodeGenContext*);
    };

    class Node : public ParserArenaFreeable {
    protected:
        Node(CodeGenContext *context, OpcodeID opcodeID);

    public:
        virtual ~Node() { }
        virtual RegisterID* emitBytecode(CodeGenContext&, RegisterID* destination = 0) = 0;

        bool isNumber() const { return (op_const_number == m_opcodeID); }
        bool isString() const { return (op_const_string == m_opcodeID); }
        bool isNull() const { return (op_const_null == m_opcodeID); }
        bool isAdd() const { return (op_add == m_opcodeID); }
        bool isResolveNode() const { return (op_resolve == m_opcodeID); }
        bool isCommaNode() const { return (op_resolve_list == m_opcodeID); }
        bool isReturnNode() const { return (op_ret == m_opcodeID); }
        bool isConstant(double &result) const;
        virtual bool isPure(CodeGenContext&) const { return false; }        
        virtual bool isLocation() const { return false; }
        virtual bool isBracketAccessorNode() const { return false; }
        virtual bool isDotAccessorNode() const { return false; }
        virtual bool isFuncExprNode() const { return false; }
        virtual bool isSimpleArray() const { return false; }

        virtual bool isEmptyStatement() const { return false; }
        virtual bool isExprStatement() const { return false; }

        virtual bool isBlock() const { return false; }

        OpcodeID opcodeID() const { return m_opcodeID; }
        int lineNo() const { return m_line; }

    protected:
        OpcodeID m_opcodeID;
        int m_line;
    };

    class ExpressionNode : public Node {
    protected:
        ExpressionNode(CodeGenContext *context, OpcodeID opcodeID = op_end, ResultType resultType = ResultType::unknownType());

    public:
        virtual ExpressionNode* stripUnaryPlus() { return this; }

        ResultType resultDescriptor() const { return m_resultType; }

    private:
        ResultType m_resultType;
    };

    class StatementNode : public Node {
    protected:
        StatementNode(CodeGenContext *context, OpcodeID opcodeID = op_end);

    public:
        void setLoc(int firstLine, int lastLine);
        int firstLine() const { return lineNo(); }
        int lastLine() const { return m_lastLine; }

    private:
        int m_lastLine;
    };

    class IdentifyNode : public ExpressionNode {
    public:
        IdentifyNode(CodeGenContext*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class NullNode : public ExpressionNode {
    public:
        NullNode(CodeGenContext*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class BooleanNode : public ExpressionNode {
    public:
        BooleanNode(CodeGenContext *context, bool value);

        bool value() const { return m_value; }

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        virtual bool isPure(CodeGenContext&) const { return true; }

        bool m_value;
    };

    class NumberNode : public ExpressionNode {
    public:
        NumberNode(CodeGenContext *context, double value);

        double value() const { return m_value; }
        void setValue(double value) { m_value = value; }

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        virtual bool isPure(CodeGenContext&) const { return true; }

        double m_value;
    };

    class StringNode : public ExpressionNode {
    public:
        StringNode(CodeGenContext *context, const Identifier&);

        const Identifier& value() { return m_value; }

    private:
        virtual bool isPure(CodeGenContext&) const { return true; }

        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        const Identifier& m_value;
    };
    
    class ThrowableExpressionData {
    public:
        ThrowableExpressionData()
            : m_divot(static_cast<uint32_t>(-1))
            , m_startOffset(static_cast<uint16_t>(-1))
            , m_endOffset(static_cast<uint16_t>(-1))
        {
        }
        
        ThrowableExpressionData(unsigned divot, unsigned startOffset, unsigned endOffset)
            : m_divot(divot)
            , m_startOffset(startOffset)
            , m_endOffset(endOffset)
        {
        }
        
        void setExceptionSourceCode(unsigned divot, unsigned startOffset, unsigned endOffset)
        {
            m_divot = divot;
            m_startOffset = startOffset;
            m_endOffset = endOffset;
        }

        uint32_t divot() const { return m_divot; }
        uint16_t startOffset() const { return m_startOffset; }
        uint16_t endOffset() const { return m_endOffset; }

    protected:
        RegisterID* emitThrowError(CodeGenContext&, ErrorType, const char* message);
        RegisterID* emitThrowError(CodeGenContext&, ErrorType, const char* message, const Identifier&);

    private:
        uint32_t m_divot;
        uint16_t m_startOffset;
        uint16_t m_endOffset;
    };

    class ThrowableSubExpressionData : public ThrowableExpressionData {
    public:
        ThrowableSubExpressionData()
            : m_subexpressionDivotOffset(0)
            , m_subexpressionEndOffset(0)
        {
        }

        ThrowableSubExpressionData(unsigned divot, unsigned startOffset, unsigned endOffset)
            : ThrowableExpressionData(divot, startOffset, endOffset)
            , m_subexpressionDivotOffset(0)
            , m_subexpressionEndOffset(0)
        {
        }

        void setSubexpressionInfo(uint32_t subexpressionDivot, uint16_t subexpressionOffset)
        {
            ASSERT(subexpressionDivot <= divot());
            if ((divot() - subexpressionDivot) & ~0xFFFF) // Overflow means we can't do this safely, so just point at the primary divot
                return;
            m_subexpressionDivotOffset = divot() - subexpressionDivot;
            m_subexpressionEndOffset = subexpressionOffset;
        }

    protected:
        uint16_t m_subexpressionDivotOffset;
        uint16_t m_subexpressionEndOffset;
    };
    
    class ThrowablePrefixedSubExpressionData : public ThrowableExpressionData {
    public:
        ThrowablePrefixedSubExpressionData()
            : m_subexpressionDivotOffset(0)
            , m_subexpressionStartOffset(0)
        {
        }

        ThrowablePrefixedSubExpressionData(unsigned divot, unsigned startOffset, unsigned endOffset)
            : ThrowableExpressionData(divot, startOffset, endOffset)
            , m_subexpressionDivotOffset(0)
            , m_subexpressionStartOffset(0)
        {
        }

        void setSubexpressionInfo(uint32_t subexpressionDivot, uint16_t subexpressionOffset)
        {
            ASSERT(subexpressionDivot >= divot());
            if ((subexpressionDivot - divot()) & ~0xFFFF) // Overflow means we can't do this safely, so just point at the primary divot
                return;
            m_subexpressionDivotOffset = subexpressionDivot - divot();
            m_subexpressionStartOffset = subexpressionOffset;
        }

    protected:
        uint16_t m_subexpressionDivotOffset;
        uint16_t m_subexpressionStartOffset;
    };

    class RegExpNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        RegExpNode(CodeGenContext *context, const Identifier& pattern, const Identifier& flags);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        const Identifier& m_pattern;
        const Identifier& m_flags;
    };

    class ThisNode : public ExpressionNode {
    public:
        ThisNode(CodeGenContext*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class ResolveNode : public ExpressionNode {
    public:
        ResolveNode(CodeGenContext *context, const Identifier &ident, int startOffset);

        RegisterID* createVariable(CodeGenContext &context, bool inLocal = true);
        const Identifier& identifier() const { return m_ident; }

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        virtual bool isPure(CodeGenContext&) const ;
        virtual bool isLocation() const { return true; }

        const Identifier& m_ident;
        int32_t m_startOffset;
    };

    class ElementNode : public ParserArenaFreeable, public NodeList<ElementNode> {
    public:
        ElementNode(CodeGenContext *context, int elision, ExpressionNode *node, ElementNode *last = NULL);

        int elision() const { return m_elision; }
        ExpressionNode* value() { return m_node; }

    private:
        int m_elision;
        ExpressionNode* m_node;
    };

    class ArrayNode : public ExpressionNode {
    public:
        ArrayNode(CodeGenContext *context, int elision, ElementNode *element = NULL);

        ArgumentListNode* toArgumentList(CodeGenContext*) const;

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        virtual bool isSimpleArray() const ;

        ElementNode* m_element;
        int m_elision;
        bool m_optional;
    };

    class PropertyNode : public ParserArenaFreeable {
    public:
        enum Type { Constant, Getter, Setter };

        PropertyNode(CodeGenContext *globalData, const Identifier& ident, ExpressionNode* assign, Type type);
        PropertyNode(CodeGenContext *globalData, double number, ExpressionNode* assign, Type type);

        const Identifier& identifier() const { return m_ident; }
        const double number() const { return m_number; }

    private:
        friend class PropertyListNode;
        const Identifier m_ident;
        const double m_number;
        ExpressionNode* m_assign;
        Type m_type;
    };

    class PropertyListNode : public Node, public NodeList<PropertyListNode> {
    public:
        PropertyListNode(CodeGenContext *context, PropertyNode *node, PropertyListNode *last = NULL);

        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

    private:
        PropertyNode* m_node;
    };

    class ObjectLiteralNode : public ExpressionNode {
    public:
        ObjectLiteralNode(CodeGenContext *context, PropertyListNode *list = NULL);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        PropertyListNode* m_list;
    };
    
    class BracketAccessorNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        BracketAccessorNode(CodeGenContext *context, ExpressionNode* base, ExpressionNode* subscript, bool subscriptHasAssignments);

        ExpressionNode* base() const { return m_base; }
        ExpressionNode* subscript() const { return m_subscript; }

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        virtual bool isLocation() const { return true; }
        virtual bool isBracketAccessorNode() const { return true; }

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
        bool m_subscriptHasAssignments;
    };

    class DotAccessorNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        DotAccessorNode(CodeGenContext *context, ExpressionNode* base, const Identifier&);

        ExpressionNode* base() const { return m_base; }
        const Identifier& identifier() const { return m_ident; }

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        virtual bool isLocation() const { return true; }
        virtual bool isDotAccessorNode() const { return true; }

        ExpressionNode* m_base;
        const Identifier& m_ident;
    };

    class ArgumentListNode : public Node, public NodeList<ArgumentListNode> {
    public:
        ArgumentListNode(CodeGenContext *context, ExpressionNode *expr, ArgumentListNode *last = NULL);

        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

    private:
        ExpressionNode* m_expr;
    };

    class ArgumentsNode : public ParserArenaFreeable {
    public:
        ArgumentsNode(CodeGenContext*);
        ArgumentsNode(CodeGenContext *context, ArgumentListNode*);

        ArgumentListNode* m_listNode;
    };

    class NewExprNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        NewExprNode(CodeGenContext *context, ExpressionNode*);
        NewExprNode(CodeGenContext *context, ExpressionNode*, ArgumentsNode*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
        ArgumentsNode* m_args;
    };

    class EvalFunctionCallNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        EvalFunctionCallNode(CodeGenContext *context, ArgumentsNode*, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ArgumentsNode* m_args;
    };

    class FunctionCallValueNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        FunctionCallValueNode(CodeGenContext* globalData, ExpressionNode* expr, ArgumentsNode* args, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
        ArgumentsNode* m_args;
    };

    class FunctionCallResolveNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        FunctionCallResolveNode(CodeGenContext *context, const Identifier &ident, ArgumentsNode*, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        const Identifier& m_ident;
        ArgumentsNode* m_args;
        size_t m_index; // Used by LocalVarFunctionCallNode.
        size_t m_scopeDepth; // Used by ScopedVarFunctionCallNode and NonLocalVarFunctionCallNode
    };
    
    class FunctionCallBracketNode : public ExpressionNode, public ThrowableSubExpressionData {
    public:
        FunctionCallBracketNode(CodeGenContext *context, ExpressionNode* base, ExpressionNode* subscript, ArgumentsNode*, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
        ArgumentsNode* m_args;
    };

    class FunctionCallDotNode : public ExpressionNode, public ThrowableSubExpressionData {
    public:
        FunctionCallDotNode(CodeGenContext *context, ExpressionNode* base, const Identifier &ident, ArgumentsNode*, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

    protected:
        ExpressionNode* m_base;
        const Identifier& m_ident;
        ArgumentsNode* m_args;
    };

    class CallFunctionCallDotNode : public FunctionCallDotNode {
    public:
        CallFunctionCallDotNode(CodeGenContext *context, ExpressionNode* base, const Identifier &ident, ArgumentsNode*, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };
    
    class ApplyFunctionCallDotNode : public FunctionCallDotNode {
    public:
        ApplyFunctionCallDotNode(CodeGenContext *context, ExpressionNode* base, const Identifier &ident, ArgumentsNode*, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class PrePostResolveNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        PrePostResolveNode(CodeGenContext *context, OpcodeID opcodeID, const Identifier &ident, unsigned divot, unsigned startOffset, unsigned endOffset);

    protected:
        const Identifier& m_ident;
    };

    class PostfixResolveNode : public PrePostResolveNode {
    public:
        PostfixResolveNode(CodeGenContext *context, const Identifier& ident, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class PostfixBracketNode : public ExpressionNode, public ThrowableSubExpressionData {
    public:
        PostfixBracketNode(CodeGenContext *context, ExpressionNode* base, ExpressionNode* subscript, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
    };

    class PostfixDotNode : public ExpressionNode, public ThrowableSubExpressionData {
    public:
        PostfixDotNode(CodeGenContext *context, ExpressionNode* base, const Identifier &ident, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_base;
        const Identifier& m_ident;
    };

    class PostfixErrorNode : public ExpressionNode, public ThrowableSubExpressionData {
    public:
        PostfixErrorNode(CodeGenContext *context, ExpressionNode*, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
    };

    class DeleteResolveNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        DeleteResolveNode(CodeGenContext *context, const Identifier &ident, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        const Identifier& m_ident;
    };

    class DeleteBracketNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        DeleteBracketNode(CodeGenContext *context, ExpressionNode* base, ExpressionNode* subscript, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
    };

    class DeleteDotNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        DeleteDotNode(CodeGenContext *context, ExpressionNode* base, const Identifier &ident, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_base;
        const Identifier& m_ident;
    };

    class DeleteValueNode : public ExpressionNode {
    public:
        DeleteValueNode(CodeGenContext *context, ExpressionNode*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
    };

    class VoidNode : public ExpressionNode {
    public:
        VoidNode(CodeGenContext *context, ExpressionNode*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
    };

    class TypeOfResolveNode : public ExpressionNode {
    public:
        TypeOfResolveNode(CodeGenContext *context, const Identifier&);

        const Identifier& identifier() const { return m_ident; }

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        const Identifier& m_ident;
    };

    class TypeOfValueNode : public ExpressionNode {
    public:
        TypeOfValueNode(CodeGenContext *context, ExpressionNode*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
    };

    class PrefixResolveNode : public PrePostResolveNode {
    public:
        PrefixResolveNode(CodeGenContext *context, const Identifier &ident, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class PrefixBracketNode : public ExpressionNode, public ThrowablePrefixedSubExpressionData {
    public:
        PrefixBracketNode(CodeGenContext *context, ExpressionNode* base, ExpressionNode* subscript, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
    };

    class PrefixDotNode : public ExpressionNode, public ThrowablePrefixedSubExpressionData {
    public:
        PrefixDotNode(CodeGenContext *context, ExpressionNode* base, const Identifier &ident, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_base;
        const Identifier& m_ident;
    };

    class PrefixErrorNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        PrefixErrorNode(CodeGenContext *context, ExpressionNode*, OpcodeID opcodeID, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
    };

    class UnaryOpNode : public ExpressionNode {
    public:
        UnaryOpNode(CodeGenContext *context, ResultType, ExpressionNode*, OpcodeID);
        ExpressionNode* expr() { return m_expr; }

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
    };

    class UnaryPlusNode : public UnaryOpNode {
    public:
        UnaryPlusNode(CodeGenContext *context, ExpressionNode*);

    private:
        virtual ExpressionNode* stripUnaryPlus() { return expr(); }
    };

    class NegateNode : public UnaryOpNode {
    public:
        NegateNode(CodeGenContext *context, ExpressionNode*);
    };

    class BitwiseNotNode : public UnaryOpNode {
    public:
        BitwiseNotNode(CodeGenContext *context, ExpressionNode*);
    };

    class LogicalNotNode : public UnaryOpNode {
    public:
        LogicalNotNode(CodeGenContext *context, ExpressionNode*);
    };

    class BinaryOpNode : public ExpressionNode {
    public:
        BinaryOpNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID, bool rightHasAssignments);
        BinaryOpNode(CodeGenContext *context, ResultType resultType, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID, bool rightHasAssignments);
        ExpressionNode *expr1() { return m_expr1; }
        ExpressionNode *expr2() { return m_expr2; }

        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

    protected:
        ExpressionNode* m_expr1;
        ExpressionNode* m_expr2;
    protected:
        bool m_rightHasAssignments;
    };

    class ReverseBinaryOpNode : public BinaryOpNode {
    public:
        ReverseBinaryOpNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID, bool rightHasAssignments);
        ReverseBinaryOpNode(CodeGenContext *context, ResultType, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID, bool rightHasAssignments);

        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class MultNode : public BinaryOpNode {
    public:
        MultNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class DivNode : public BinaryOpNode {
    public:
        DivNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class ModNode : public BinaryOpNode {
    public:
        ModNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class AddNode : public BinaryOpNode {
    public:
        AddNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class SubNode : public BinaryOpNode {
    public:
        SubNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class LeftShiftNode : public BinaryOpNode {
    public:
        LeftShiftNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class RightShiftNode : public BinaryOpNode {
    public:
        RightShiftNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class UnsignedRightShiftNode : public BinaryOpNode {
    public:
        UnsignedRightShiftNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class LessNode : public BinaryOpNode {
    public:
        LessNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class GreaterNode : public ReverseBinaryOpNode {
    public:
        GreaterNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class LessEqNode : public BinaryOpNode {
    public:
        LessEqNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class GreaterEqNode : public ReverseBinaryOpNode {
    public:
        GreaterEqNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class ThrowableBinaryOpNode : public BinaryOpNode, public ThrowableExpressionData {
    public:
        ThrowableBinaryOpNode(CodeGenContext *context, ResultType, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID, bool rightHasAssignments);
        ThrowableBinaryOpNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID, bool rightHasAssignments);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };
    
    class InstanceOfNode : public ThrowableBinaryOpNode {
    public:
        InstanceOfNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class InNode : public ThrowableBinaryOpNode {
    public:
        InNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class EqualNode : public BinaryOpNode {
    public:
        EqualNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class NotEqualNode : public BinaryOpNode {
    public:
        NotEqualNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class StrictEqualNode : public BinaryOpNode {
    public:
        StrictEqualNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class NotStrictEqualNode : public BinaryOpNode {
    public:
        NotStrictEqualNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class BitAndNode : public BinaryOpNode {
    public:
        BitAndNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class BitOrNode : public BinaryOpNode {
    public:
        BitOrNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class BitXOrNode : public BinaryOpNode {
    public:
        BitXOrNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    // m_expr1 && m_expr2, m_expr1 || m_expr2
    class LogicalOpNode : public BinaryOpNode {
    public:
        LogicalOpNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID opcodeID);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    // The ternary operator, "m_logical ? m_expr1 : m_expr2"
    class ConditionalNode : public ExpressionNode {
    public:
        ConditionalNode(CodeGenContext *context, ExpressionNode* logical, ExpressionNode* expr1, ExpressionNode* expr2);
        ExpressionNode *logical() { return m_logical; }
        ExpressionNode *expr1() { return m_expr1; }
        ExpressionNode *expr2() { return m_expr2; }

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_logical;
        ExpressionNode* m_expr1;
        ExpressionNode* m_expr2;
    };

    class ReadModifyResolveNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        ReadModifyResolveNode(CodeGenContext *context, const Identifier &ident, OpcodeID opcodeID, ExpressionNode*  right, bool rightHasAssignments, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        const Identifier& m_ident;
        ExpressionNode* m_right;
        size_t m_index; // Used by ReadModifyLocalVarNode.
        bool m_rightHasAssignments;
    };

    class AssignResolveNode : public ResolveNode, public ThrowableExpressionData {
    public:
        AssignResolveNode(CodeGenContext *context, const Identifier &ident, ExpressionNode* right, bool rightHasAssignments);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_right;
        size_t m_index; // Used by ReadModifyLocalVarNode.
        bool m_rightHasAssignments;
    };

    class ReadModifyBracketNode : public ExpressionNode, public ThrowableSubExpressionData {
    public:
        ReadModifyBracketNode(CodeGenContext *context, ExpressionNode* base, ExpressionNode* subscript, OpcodeID opcodeID, ExpressionNode* right, bool subscriptHasAssignments, bool rightHasAssignments, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
        ExpressionNode* m_right;
        bool m_subscriptHasAssignments;
        bool m_rightHasAssignments;
    };

    class AssignBracketNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        AssignBracketNode(CodeGenContext *context, ExpressionNode* base, ExpressionNode* subscript, ExpressionNode* right, bool subscriptHasAssignments, bool rightHasAssignments, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
        ExpressionNode* m_right;
        bool m_subscriptHasAssignments;
        bool m_rightHasAssignments;
    };

    class AssignDotNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        AssignDotNode(CodeGenContext *context, ExpressionNode* base, const Identifier &ident, ExpressionNode* right, bool rightHasAssignments, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_base;
        const Identifier& m_ident;
        ExpressionNode* m_right;
        bool m_rightHasAssignments;
    };

    class ReadModifyDotNode : public ExpressionNode, public ThrowableSubExpressionData {
    public:
        ReadModifyDotNode(CodeGenContext *context, ExpressionNode* base, const Identifier &ident, OpcodeID opcodeID, ExpressionNode* right, bool rightHasAssignments, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_base;
        const Identifier& m_ident;
        ExpressionNode* m_right;
        bool m_rightHasAssignments;
    };

    class AssignErrorNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        AssignErrorNode(CodeGenContext *context, ExpressionNode* left, OpcodeID opcodeID, ExpressionNode* right, unsigned divot, unsigned startOffset, unsigned endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_left;
        ExpressionNode* m_right;
    };
    
    typedef std::vector<ExpressionNode*> ExpressionVector;

    class CommaNode : public ExpressionNode, public ParserArenaDeletable {
    public:
        CommaNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2);

        void append(ExpressionNode* expr) { m_expressions.push_back(expr); }
        ExpressionVector &list() { return m_expressions; }
        
    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionVector m_expressions;
    };
    
    class ConstDeclNode : public ExpressionNode, public NodeList<ConstDeclNode> {
    public:
        ConstDeclNode(CodeGenContext *context, const Identifier &ident, ExpressionNode *init);

        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        bool hasInitializer() const { return m_init; }
        const Identifier& ident() { return m_ident; }

    private:
        virtual RegisterID* createConstant(CodeGenContext &context, bool inLocal = true);

        const Identifier& m_ident;
        ExpressionNode* m_init;
    };

    class ConstStatementNode : public StatementNode, public NodeList<ConstStatementNode> {
    public:
        ConstStatementNode(CodeGenContext *context, ConstDeclNode* list);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ConstDeclNode* m_list;
    };

    class SourceElements : public Node {
    public:
        SourceElements(CodeGenContext*);

        void append(StatementNode*);

        StatementNode* singleStatement() const;
        StatementNode* lastStatement() const;

        virtual RegisterID* emitBytecode(CodeGenContext&, RegisterID* destination);

    private:
        std::vector<StatementNode*> m_statements;
    };

    class BlockNode : public StatementNode {
    public:
        BlockNode(CodeGenContext *context, SourceElements* = 0);

        StatementNode* lastStatement() const;

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        virtual bool isBlock() const { return true; }

        SourceElements* m_statements;
    };

    class EmptyStatementNode : public StatementNode {
    public:
        EmptyStatementNode(CodeGenContext*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        virtual bool isEmptyStatement() const { return true; }
    };
    
    class DebuggerStatementNode : public StatementNode {
    public:
        DebuggerStatementNode(CodeGenContext*);
        
    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class ExprStatementNode : public StatementNode {
    public:
        ExprStatementNode(CodeGenContext *context, ExpressionNode*);

        ExpressionNode* expr() const { return m_expr; }

    private:
        virtual bool isExprStatement() const { return true; }

        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
    };

    class VarStatementNode : public StatementNode {
    public:
        VarStatementNode(CodeGenContext *context, ExpressionNode *expr);        

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode *m_expr;
    };

    class IfNode : public StatementNode {
    public:
        IfNode(CodeGenContext *context, ExpressionNode* condition, StatementNode* ifBlock);

    protected:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_condition;
        StatementNode* m_ifBlock;
    };

    class IfElseNode : public IfNode {
    public:
        IfElseNode(CodeGenContext *context, ExpressionNode* condition, StatementNode* ifBlock, StatementNode* elseBlock);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        StatementNode* m_elseBlock;
    };

    class DoWhileNode : public StatementNode {
    public:
        DoWhileNode(CodeGenContext *context, StatementNode* statement, ExpressionNode*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        StatementNode* m_statement;
        ExpressionNode* m_expr;
    };

    class WhileNode : public StatementNode {
    public:
        WhileNode(CodeGenContext *context, ExpressionNode*, StatementNode* statement);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
        StatementNode* m_statement;
    };

    class ForNode : public StatementNode {
    public:
        ForNode(CodeGenContext *context, ExpressionNode* expr1, ExpressionNode* expr2, ExpressionNode* expr3, StatementNode* statement, bool expr1WasVarDecl);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr1;
        ExpressionNode* m_expr2;
        ExpressionNode* m_expr3;
        StatementNode* m_statement;
        bool m_expr1WasVarDecl;
    };

    class ForInNode : public StatementNode, public ThrowableExpressionData {
    public:
        ForInNode(CodeGenContext* globalData, ExpressionNode* l, ExpressionNode* expr, StatementNode* statement);
        ForInNode(CodeGenContext* globalData, const Identifier& ident, ExpressionNode* in, ExpressionNode* expr, StatementNode* statement, int divot, int startOffset, int endOffset);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        const Identifier& m_ident;
        ExpressionNode* m_init;
        ExpressionNode* m_lexpr;
        ExpressionNode* m_expr;
        StatementNode* m_statement;
        bool m_identIsVarDecl;
    };

    class ContinueNode : public StatementNode, public ThrowableExpressionData {
    public:
        ContinueNode(CodeGenContext*);
        ContinueNode(CodeGenContext *context, const Identifier&);
        
    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        const Identifier& m_ident;
    };

    class BreakNode : public StatementNode, public ThrowableExpressionData {
    public:
        BreakNode(CodeGenContext*);
        BreakNode(CodeGenContext *context, const Identifier&);
        
    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        const Identifier& m_ident;
    };

    class ReturnNode : public StatementNode, public ThrowableExpressionData {
    public:
        ReturnNode(CodeGenContext *context, ExpressionNode* value);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_value;
    };

    class WithNode : public StatementNode {
    public:
        WithNode(CodeGenContext *context, ExpressionNode*, StatementNode*, uint32_t divot, uint32_t expressionLength);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
        StatementNode* m_statement;
        uint32_t m_divot;
        uint32_t m_expressionLength;
    };

    class LabelNode : public StatementNode, public ThrowableExpressionData {
    public:
        LabelNode(CodeGenContext *context, const Identifier& name, StatementNode*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        const Identifier& m_name;
        StatementNode* m_statement;
    };

    class ThrowNode : public StatementNode, public ThrowableExpressionData {
    public:
        ThrowNode(CodeGenContext *context, ExpressionNode*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
    };

    class TryNode : public StatementNode {
    public:
        TryNode(CodeGenContext *context, StatementNode* tryBlock, const Identifier& exceptionIdent, bool catchHasEval, StatementNode* catchBlock, StatementNode* finallyBlock);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        StatementNode* m_tryBlock;
        const Identifier& m_exceptionIdent;
        StatementNode* m_catchBlock;
        StatementNode* m_finallyBlock;
        bool m_catchHasEval;
    };

    class ParameterNode : public ParserArenaFreeable, public NodeList<ParameterNode> {
    public:
        ParameterNode(CodeGenContext *context, const Identifier &ident, ParameterNode *last = NULL);

        const Identifier& ident() const { return m_ident; }

    private:
        const Identifier& m_ident;
    };

    struct ScopeNodeData {
        typedef DeclarationStacks::VarStack VarStack;
        typedef DeclarationStacks::FunctionStack FunctionStack;

        ScopeNodeData(VarStack*, FunctionStack*, int numConstants);

        VarStack m_varStack;
        FunctionStack m_functionStack;
        int m_numConstants;
        SourceElements* m_statements;
    };

    class ScopeNode : public StatementNode {
    public:
        typedef DeclarationStacks::VarStack VarStack;
        typedef DeclarationStacks::FunctionStack FunctionStack;

        ScopeNode(CodeGenContext*);
        ScopeNode(CodeGenContext *context, SourceElements*, VarStack*, FunctionStack*, CodeFeatures, int numConstants);

        void adoptData(ScopeNodeData* data)
        {
            destroyData();
            m_data = data;
        }
        ScopeNodeData* data() const { return m_data; }
        void destroyData() { delete m_data; m_data = NULL; }

        void setFeatures(CodeFeatures features) { m_features = features; }
        CodeFeatures features() { return m_features; }

        bool usesEval() const { return m_features & EvalFeature; }
        bool usesArguments() const { return m_features & ArgumentsFeature; }
        void setUsesArguments() { m_features |= ArgumentsFeature; }
        bool usesThis() const { return m_features & ThisFeature; }
        bool needsActivation() const { return m_features & (EvalFeature | ClosureFeature | WithFeature | CatchFeature); }

        VarStack& varStack() { ASSERT(m_data); return m_data->m_varStack; }
        FunctionStack& functionStack() { ASSERT(m_data); return m_data->m_functionStack; }

        int neededConstants()
        {
            // We may need 2 more constants than the count given by the parser,
            // because of the various uses of _ejs_undefined() and _ejs_null().
            return m_data->m_numConstants + 2;
        }

        StatementNode* singleStatement() const;

        void emitStatementsBytecode(CodeGenContext&, RegisterID* destination);

    protected:

    private:
        ScopeNodeData* m_data;
        CodeFeatures m_features;
    };

    class ProgramNode : public ScopeNode {
    public:
        static ProgramNode* create(CodeGenContext *context, SourceElements*, VarStack*, FunctionStack*, CodeFeatures, int numConstants);

        static const bool scopeIsFunction = false;

    private:
        ProgramNode(CodeGenContext *context, SourceElements*, VarStack*, FunctionStack*, CodeFeatures, int numConstants);

        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class EvalNode : public ScopeNode {
    public:
        static EvalNode* create(CodeGenContext *context, SourceElements*, VarStack*, FunctionStack*, CodeFeatures, int numConstants);

        static const bool scopeIsFunction = false;

    private:
        EvalNode(CodeGenContext *context, SourceElements*, VarStack*, FunctionStack*, CodeFeatures, int numConstants);

        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);
    };

    class FunctionParameters : public std::vector<Identifier> {
    public:
        static FunctionParameters* create(ParameterNode* firstParameter) { return new FunctionParameters(firstParameter); }

    private:
        FunctionParameters(ParameterNode*);
    };

    class FunctionBodyNode : public ScopeNode {
    public:
        static FunctionBodyNode* create(CodeGenContext* globalData, SourceElements* children = NULL, VarStack* varStack = NULL, FunctionStack* funcStack = NULL, CodeFeatures features = 0, int numConstants = 0);

        FunctionParameters* parameters() const { return m_parameters; }
        size_t parameterCount() const { return m_parameters->size(); }

        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        void finishParsing(ParameterNode*, const Identifier&);
        void finishParsing(FunctionParameters*, const Identifier&);
        
        const Identifier& ident() { return m_ident; }

        static const bool scopeIsFunction = true;

    private:
        FunctionBodyNode(CodeGenContext* globalData, SourceElements* children = NULL, VarStack* varStack = NULL, FunctionStack* funcStack = NULL, CodeFeatures features = 0, int numConstants = 0);

        Identifier m_ident;
        FunctionParameters* m_parameters;
    };

    class FuncExprNode : public ExpressionNode {
    public:
        FuncExprNode(CodeGenContext *context, const Identifier &ident, FunctionBodyNode* body, ParameterNode* parameter = 0);

        FunctionBodyNode* body() { return m_body; }

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        virtual bool isFuncExprNode() const { return true; } 

        FunctionBodyNode* m_body;
    };

    class FuncDeclNode : public StatementNode {
    public:
        FuncDeclNode(CodeGenContext *context, const Identifier &ident, FunctionBodyNode* body, ParameterNode* parameter = 0);

        FunctionBodyNode* body() { return m_body; }

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        FunctionBodyNode* m_body;
    };

    class CaseClauseNode : public ParserArenaFreeable {
    public:
        CaseClauseNode(CodeGenContext *context, ExpressionNode*, SourceElements* = 0);

        ExpressionNode* expr() const { return m_expr; }
        SourceElements* statments() const { return m_statements; }

    private:
        ExpressionNode* m_expr;
        SourceElements* m_statements;
    };

    class ClauseListNode : public ParserArenaFreeable, public NodeList<ClauseListNode> {
    public:
        ClauseListNode(CodeGenContext *context, CaseClauseNode *clause, ClauseListNode *last = NULL);

        CaseClauseNode* getClause() const { return m_clause; }

    private:
        CaseClauseNode* m_clause;
    };

    class CaseBlockNode : public ParserArenaFreeable {
    public:
        CaseBlockNode(CodeGenContext *context, ClauseListNode* list1, CaseClauseNode* defaultClause, ClauseListNode* list2);

        ClauseListNode* list1() { return m_list1; }
        ClauseListNode* list2() { return m_list2; }
        CaseClauseNode* defaultClause() { return m_defaultClause; }

    private:
        ClauseListNode* m_list1;
        CaseClauseNode* m_defaultClause;
        ClauseListNode* m_list2;
    };

    class SwitchNode : public StatementNode {
    public:
        SwitchNode(CodeGenContext *context, ExpressionNode*, CaseBlockNode*);

    private:
        virtual RegisterID* emitBytecode(CodeGenContext &context, RegisterID* dst = NULL);

        ExpressionNode* m_expr;
        CaseBlockNode* m_block;
    };

    struct ElementList {
        ElementNode* head;
        ElementNode* tail;
    };

    struct PropertyList {
        PropertyListNode* head;
        PropertyListNode* tail;
    };

    struct ArgumentList {
        ArgumentListNode* head;
        ArgumentListNode* tail;
    };

    struct ConstDeclList {
        ConstDeclNode* head;
        ConstDeclNode* tail;
    };

    struct ParameterList {
        ParameterNode* head;
        ParameterNode* tail;
    };

    struct ClauseList {
        ClauseListNode* head;
        ClauseListNode* tail;
    };

} // namespace JSC
using namespace JSC;
#endif // Nodes_h

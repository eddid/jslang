%{

/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 *  Copyright (C) 2014 Eddid Zhang <zhangheng607@163.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "NodeInfo.h"
#include "CodeGen.h"
#include "NodeConstructors.h"
#include <stdlib.h>
#include <string.h>

#define YYMALLOC malloc
#define YYFREE free

#define YYMAXDEPTH 10000
#define YYENABLE_NLS 0

// Default values for bison.
#define YYDEBUG 1 // Set to 1 to debug a parse error.
#define jscyydebug 1 // Set to 1 to debug a parse error.
#if !PLATFORM(DARWIN)
// Avoid triggering warnings in older bison by not setting this on the Darwin platform.
// FIXME: Is this still needed?
#define YYERROR_VERBOSE
#endif

extern int yylex(void *globalPtr);
extern int yyerror(const char*);
SourceElements *programBlock = NULL; /* the top level root node of our final AST */

#define GLOBAL_DATA static_cast<CodeGenContext*>(globalPtr)
#define AUTO_SEMICOLON do { ; } while (0)

using namespace JSC;
using namespace std;

static ExpressionNode* makeAssignNode(CodeGenContext*, ExpressionNode* left, OpcodeID, ExpressionNode* right, bool leftHasAssignments, bool rightHasAssignments, int start, int divot, int end);
static ExpressionNode* makePrefixNode(CodeGenContext*, ExpressionNode*, OpcodeID, int start, int divot, int end);
static ExpressionNode* makePostfixNode(CodeGenContext*, ExpressionNode*, OpcodeID, int start, int divot, int end);
static PropertyNode* makeGetterOrSetterPropertyNode(CodeGenContext*, const Identifier& getOrSet, const Identifier& name, ParameterNode*, FunctionBodyNode*);
static ExpressionNodeInfo makeFunctionCallNode(CodeGenContext*, ExpressionNodeInfo function, ArgumentsNodeInfo, int start, int divot, int end);
static ExpressionNode* makeTypeOfNode(CodeGenContext*, ExpressionNode*);
static ExpressionNode* makeDeleteNode(CodeGenContext*, ExpressionNode*, int start, int divot, int end);
static ExpressionNode* makeNegateNode(CodeGenContext*, ExpressionNode*);
static NumberNode* makeNumberNode(CodeGenContext*, double);
static ExpressionNode* makeBitwiseNotNode(CodeGenContext*, ExpressionNode*);
static ExpressionNode* makeMultNode(CodeGenContext*, ExpressionNode* left, ExpressionNode* right, bool rightHasAssignments);
static ExpressionNode* makeDivNode(CodeGenContext*, ExpressionNode* left, ExpressionNode* right, bool rightHasAssignments);
static ExpressionNode* makeAddNode(CodeGenContext*, ExpressionNode* left, ExpressionNode* right, bool rightHasAssignments);
static ExpressionNode* makeSubNode(CodeGenContext*, ExpressionNode* left, ExpressionNode* right, bool rightHasAssignments);
static ExpressionNode* makeLeftShiftNode(CodeGenContext*, ExpressionNode* left, ExpressionNode* right, bool rightHasAssignments);
static ExpressionNode* makeRightShiftNode(CodeGenContext*, ExpressionNode* left, ExpressionNode* right, bool rightHasAssignments);
static StatementNode* makeVarStatementNode(CodeGenContext*, ExpressionNode*);
static ExpressionNode* combineCommaNodes(CodeGenContext*, ExpressionNode* list, ExpressionNode* init);

#if COMPILER(MSVC)

#pragma warning(disable: 4065)
#pragma warning(disable: 4244)
#pragma warning(disable: 4702)

#endif

#define YYPARSE_PARAM globalPtr
#define YYLEX_PARAM globalPtr

template <typename T> inline NodeDeclarationInfo<T> createNodeDeclarationInfo(T node,
    ParserArenaData<DeclarationStacks::VarStack>* varDecls,
    ParserArenaData<DeclarationStacks::FunctionStack>* funcDecls,
    CodeFeatures info, int numConstants) 
{
    ASSERT((info & ~AllFeatures) == 0);
    NodeDeclarationInfo<T> result = { node, varDecls, funcDecls, info, numConstants };
    return result;
}

template <typename T> inline NodeInfo<T> createNodeInfo(T node, CodeFeatures info, int numConstants)
{
    ASSERT((info & ~AllFeatures) == 0);
    NodeInfo<T> result = { node, info, numConstants };
    return result;
}

template <typename T> inline T mergeDeclarationLists(T decls1, T decls2) 
{
    // decls1 or both are null
    if (!decls1)
        return decls2;
    // only decls1 is non-null
    if (!decls2)
        return decls1;

    // Both are non-null
	decls1->data.insert(decls1->data.end(), decls2->data.begin(), decls2->data.end());
    
    // Manually release as much as possible from the now-defunct declaration lists
    // to avoid accumulating so many unused heap allocated vectors.
    decls2->data.clear();

    return decls1;
}

static inline void appendToVarDeclarationList(CodeGenContext* globalData, ParserArenaData<DeclarationStacks::VarStack>*& varDecls, const Identifier& ident, unsigned attrs)
{
    if (!varDecls)
        varDecls = new ParserArenaData<DeclarationStacks::VarStack>;

    varDecls->data.push_back(make_pair(&ident, attrs));
}

static inline void appendToVarDeclarationList(CodeGenContext* globalData, ParserArenaData<DeclarationStacks::VarStack>*& varDecls, ConstDeclNode* decl)
{
    unsigned attrs = DeclarationStacks::IsConstant;
    if (decl->hasInitializer())
        attrs |= DeclarationStacks::HasInitializer;        
    appendToVarDeclarationList(globalData, varDecls, decl->ident(), attrs);
}

%}

%union {
    int                 intValue;
    double              doubleValue;
    const Identifier*   ident;

    // expression subtrees
    ExpressionNodeInfo  expressionNode;
    FuncDeclNodeInfo    funcDeclNode;
    PropertyNodeInfo    propertyNode;
    ArgumentsNodeInfo   argumentsNode;
    ConstDeclNodeInfo   constDeclNode;
    CaseBlockNodeInfo   caseBlockNode;
    CaseClauseNodeInfo  caseClauseNode;
    FuncExprNodeInfo    funcExprNode;

    // statement nodes
    StatementNodeInfo   statementNode;
    FunctionBodyNode*   functionBodyNode;
    ProgramNode*        programNode;

    SourceElementsInfo  sourceElements;
    PropertyListInfo    propertyList;
    ArgumentListInfo    argumentList;
    VarDeclListInfo     varDeclList;
    ConstDeclListInfo   constDeclList;
    ClauseListInfo      clauseList;
    ElementListInfo     elementList;
    ParameterListInfo   parameterList;

    OpcodeID            op;
}

%{

template <typename T> inline void setStatementLocation(StatementNode* statement, const T& start, const T& end)
{
    statement->setLoc(start.first_line, end.last_line);
}

static inline void setExceptionLocation(ThrowableExpressionData* node, unsigned start, unsigned divot, unsigned end)
{
    node->setExceptionSourceCode(divot, divot - start, end - divot);
}

%}

%start Program

/* literals */
%token NULLTOKEN TRUETOKEN FALSETOKEN

/* keywords */
%token BREAK CASE DEFAULT FOR NEW VAR CONSTTOKEN CONTINUE
%token FUNCTION RETURN VOIDTOKEN DELETETOKEN
%token IF THISTOKEN DO WHILE INTOKEN INSTANCEOF TYPEOF
%token SWITCH WITH RESERVED
%token THROW TRY CATCH FINALLY
%token DEBUGGER

/* give an if without an else higher precedence than an else to resolve the ambiguity */
%nonassoc IF_WITHOUT_ELSE
%nonassoc ELSE

/* punctuators */
%token EQEQ NE                     /* == and != */
%token STREQ STRNEQ                /* === and !== */
%token LE GE                       /* <= and >= */
%token OR AND                      /* || and && */
%token PLUSPLUS MINUSMINUS         /* ++ and --  */
%token LSHIFT                      /* << */
%token RSHIFT URSHIFT              /* >> and >>> */
%token PLUSEQUAL MINUSEQUAL        /* += and -= */
%token MULTEQUAL DIVEQUAL          /* *= and /= */
%token LSHIFTEQUAL                 /* <<= */
%token RSHIFTEQUAL URSHIFTEQUAL    /* >>= and >>>= */
%token ANDEQUAL MODEQUAL           /* &= and %= */
%token XOREQUAL OREQUAL            /* ^= and |= */
%token <intValue> OPENBRACE        /* { (with char offset) */
%token <intValue> CLOSEBRACE       /* } (with char offset) */

%token LT GT                       /* < and > */
%token EQUAL NOT                   /* = and ! */
%token BITOR BITAND                /* | and & */
%token BITXOR BITINV               /* ^ and ~ */
%token LPAREN RPAREN               /* ( and ) */
%token LBRACKET RBRACKET           /* [ and ] */
%token QUESTIONMARK DOT COLON      /* ? and . and : */
%token SEMICOLON COMMA             /* ; and , */
%token PLUS MINUS                  /* + and - */
%token MUL DIV                     /* * and / */
%token MOD                         /* % */

/* terminal types */
%token <doubleValue> NUMBER
%token <ident> IDENT STRING REGEXP

/* automatically inserted semicolon */
%token AUTOPLUSPLUS AUTOMINUSMINUS

/* non-terminal types */
%type <expressionNode>  Literal ArrayLiteral

%type <expressionNode>  PrimaryExpr PrimaryExprNoBrace
%type <expressionNode>  MemberExpr MemberExprNoBF /* BF => brace or function */
%type <expressionNode>  NewExpr NewExprNoBF
%type <expressionNode>  CallExpr CallExprNoBF
%type <expressionNode>  LeftHandSideExpr LeftHandSideExprNoBF
%type <expressionNode>  PostfixExpr PostfixExprNoBF
%type <expressionNode>  UnaryExpr UnaryExprNoBF UnaryExprCommon
%type <expressionNode>  MultiplicativeExpr MultiplicativeExprNoBF
%type <expressionNode>  AdditiveExpr AdditiveExprNoBF
%type <expressionNode>  ShiftExpr ShiftExprNoBF
%type <expressionNode>  RelationalExpr RelationalExprNoIn RelationalExprNoBF
%type <expressionNode>  EqualityExpr EqualityExprNoIn EqualityExprNoBF
%type <expressionNode>  BitwiseANDExpr BitwiseANDExprNoIn BitwiseANDExprNoBF
%type <expressionNode>  BitwiseXORExpr BitwiseXORExprNoIn BitwiseXORExprNoBF
%type <expressionNode>  BitwiseORExpr BitwiseORExprNoIn BitwiseORExprNoBF
%type <expressionNode>  LogicalANDExpr LogicalANDExprNoIn LogicalANDExprNoBF
%type <expressionNode>  LogicalORExpr LogicalORExprNoIn LogicalORExprNoBF
%type <expressionNode>  ConditionalExpr ConditionalExprNoIn ConditionalExprNoBF
%type <expressionNode>  AssignmentExpr AssignmentExprNoIn AssignmentExprNoBF
%type <expressionNode>  Expr ExprNoIn ExprNoBF

%type <expressionNode>  ExprOpt ExprNoInOpt

%type <statementNode>   Statement Block
%type <statementNode>   VariableStatement ConstStatement EmptyStatement ExprStatement
%type <statementNode>   IfStatement IterationStatement ContinueStatement
%type <statementNode>   BreakStatement ReturnStatement WithStatement
%type <statementNode>   SwitchStatement LabelledStatement
%type <statementNode>   ThrowStatement TryStatement
%type <statementNode>   DebuggerStatement

%type <expressionNode>  Initializer InitializerNoIn
%type <statementNode>   FunctionDeclaration
%type <funcExprNode>    FunctionExpr
%type <functionBodyNode> FunctionBody
%type <sourceElements>  SourceElements
%type <parameterList>   FormalParameterList
%type <op>              AssignmentOperator
%type <argumentsNode>   Arguments
%type <argumentList>    ArgumentList
%type <varDeclList>     VariableDeclarationList VariableDeclarationListNoIn
%type <constDeclList>   ConstDeclarationList
%type <constDeclNode>   ConstDeclaration
%type <caseBlockNode>   CaseBlock
%type <caseClauseNode>  CaseClause DefaultClause
%type <clauseList>      CaseClauses CaseClausesOpt
%type <intValue>        Elision ElisionOpt
%type <elementList>     ElementList
%type <propertyNode>    Property
%type <propertyList>    PropertyList
%%

// FIXME: There are currently two versions of the grammar in this file, the normal one, and the NoNodes version used for
// lazy recompilation of FunctionBodyNodes.  We should move to generating the two versions from a script to avoid bugs.
// In the mean time, make sure to make any changes to the grammar in both versions.

Literal:
    NULLTOKEN                           { $$ = createNodeInfo<ExpressionNode*>(new NullNode(GLOBAL_DATA), 0, 1); }
  | TRUETOKEN                           { $$ = createNodeInfo<ExpressionNode*>(new BooleanNode(GLOBAL_DATA, true), 0, 1); }
  | FALSETOKEN                          { $$ = createNodeInfo<ExpressionNode*>(new BooleanNode(GLOBAL_DATA, false), 0, 1); }
  | NUMBER                              { $$ = createNodeInfo<ExpressionNode*>(makeNumberNode(GLOBAL_DATA, $1), 0, 1); }
  | STRING                              { $$ = createNodeInfo<ExpressionNode*>(new StringNode(GLOBAL_DATA, *$1), 0, 1); }
  | REGEXP                              {
                                            size_t lastDIV = $1->name.rfind('/');
											const Identifier* pattern = new Identifier($1->name.c_str(), lastDIV);
											const Identifier* flags = new Identifier($1->name.c_str() + lastDIV + 1, $1->name.size() - lastDIV - 1);
                                            RegExpNode* node = new RegExpNode(GLOBAL_DATA, *pattern, *flags);
                                            setExceptionLocation(node, @1.first_column, @1.first_column + lastDIV, @1.first_column + lastDIV);
                                            $$ = createNodeInfo<ExpressionNode*>(node, 0, 0);
											delete $1;
										}
;

Property:
    IDENT COLON AssignmentExpr            { $$ = createNodeInfo<PropertyNode*>(new PropertyNode(GLOBAL_DATA, *$1, $3.m_node, PropertyNode::Constant), $3.m_features, $3.m_numConstants); }
  | STRING COLON AssignmentExpr           { $$ = createNodeInfo<PropertyNode*>(new PropertyNode(GLOBAL_DATA, *$1, $3.m_node, PropertyNode::Constant), $3.m_features, $3.m_numConstants); }
  | NUMBER COLON AssignmentExpr           { $$ = createNodeInfo<PropertyNode*>(new PropertyNode(GLOBAL_DATA, $1, $3.m_node, PropertyNode::Constant), $3.m_features, $3.m_numConstants); }
  | IDENT IDENT LPAREN RPAREN OPENBRACE FunctionBody CLOSEBRACE    { $$ = createNodeInfo<PropertyNode*>(makeGetterOrSetterPropertyNode(GLOBAL_DATA, *$1, *$2, 0, $6), ClosureFeature, 0); setStatementLocation($6, @5, @7); if (!$$.m_node) YYABORT; }
  | IDENT IDENT LPAREN FormalParameterList RPAREN OPENBRACE FunctionBody CLOSEBRACE
                                                             {
                                                                 $$ = createNodeInfo<PropertyNode*>(makeGetterOrSetterPropertyNode(GLOBAL_DATA, *$1, *$2, $4.m_node.head, $7), $4.m_features | ClosureFeature, 0); 
                                                                 if ($4.m_features & ArgumentsFeature)
                                                                     $7->setUsesArguments(); 
                                                                 setStatementLocation($7, @6, @8); 
                                                                 if (!$$.m_node) 
                                                                     YYABORT; 
                                                             }
;

PropertyList:
    Property                            { $$.m_node.head = new PropertyListNode(GLOBAL_DATA, $1.m_node); 
                                          $$.m_node.tail = $$.m_node.head;
                                          $$.m_features = $1.m_features;
                                          $$.m_numConstants = $1.m_numConstants; }
  | PropertyList COMMA Property           { $$.m_node.head = $1.m_node.head;
                                          $$.m_node.tail = new PropertyListNode(GLOBAL_DATA, $3.m_node, $1.m_node.tail);
                                          $$.m_features = $1.m_features | $3.m_features;
                                          $$.m_numConstants = $1.m_numConstants + $3.m_numConstants; }
;

PrimaryExpr:
    PrimaryExprNoBrace
  | OPENBRACE CLOSEBRACE                             { $$ = createNodeInfo<ExpressionNode*>(new ObjectLiteralNode(GLOBAL_DATA), 0, 0); }
  | OPENBRACE PropertyList CLOSEBRACE                { $$ = createNodeInfo<ExpressionNode*>(new ObjectLiteralNode(GLOBAL_DATA, $2.m_node.head), $2.m_features, $2.m_numConstants); }
  /* allow extra comma, see http://bugs.webkit.org/show_bug.cgi?id=5939 */
  | OPENBRACE PropertyList COMMA CLOSEBRACE            { $$ = createNodeInfo<ExpressionNode*>(new ObjectLiteralNode(GLOBAL_DATA, $2.m_node.head), $2.m_features, $2.m_numConstants); }
;

PrimaryExprNoBrace:
    THISTOKEN                           { $$ = createNodeInfo<ExpressionNode*>(new ThisNode(GLOBAL_DATA), ThisFeature, 0); }
  | Literal
  | ArrayLiteral
  | IDENT                               { $$ = createNodeInfo<ExpressionNode*>(new ResolveNode(GLOBAL_DATA, *$1, @1.first_column), (*$1 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0, 0); }
  | LPAREN Expr RPAREN                        { $$ = $2; }
;

ArrayLiteral:
    LBRACKET ElisionOpt RBRACKET                  { $$ = createNodeInfo<ExpressionNode*>(new ArrayNode(GLOBAL_DATA, $2), 0, $2 ? 1 : 0); }
  | LBRACKET ElementList RBRACKET                 { $$ = createNodeInfo<ExpressionNode*>(new ArrayNode(GLOBAL_DATA, 0, $2.m_node.head), $2.m_features, $2.m_numConstants); }
  | LBRACKET ElementList COMMA ElisionOpt RBRACKET  { $$ = createNodeInfo<ExpressionNode*>(new ArrayNode(GLOBAL_DATA, $4, $2.m_node.head), $2.m_features, $4 ? $2.m_numConstants + 1 : $2.m_numConstants); }
;

ElementList:
    ElisionOpt AssignmentExpr           { $$.m_node.head = new ElementNode(GLOBAL_DATA, $1, $2.m_node);
                                          $$.m_node.tail = $$.m_node.head;
                                          $$.m_features = $2.m_features;
                                          $$.m_numConstants = $2.m_numConstants; }
  | ElementList COMMA ElisionOpt AssignmentExpr
                                        { $$.m_node.head = $1.m_node.head;
                                          $$.m_node.tail = new ElementNode(GLOBAL_DATA, $3, $4.m_node, $1.m_node.tail);
                                          $$.m_features = $1.m_features | $4.m_features;
                                          $$.m_numConstants = $1.m_numConstants + $4.m_numConstants; }
;

ElisionOpt:
    /* nothing */                       { $$ = 0; }
  | Elision
;

Elision:
    COMMA                                 { $$ = 1; }
  | Elision COMMA                         { $$ = $1 + 1; }
;

MemberExpr:
    PrimaryExpr
  | FunctionExpr                        { $$ = createNodeInfo<ExpressionNode*>($1.m_node, $1.m_features, $1.m_numConstants); }
  | MemberExpr LBRACKET Expr RBRACKET             { BracketAccessorNode* node = new BracketAccessorNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @4.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); 
                                        }
  | MemberExpr DOT IDENT                { DotAccessorNode* node = new DotAccessorNode(GLOBAL_DATA, $1.m_node, *$3);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @3.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features, $1.m_numConstants);
                                        }
  | NEW MemberExpr Arguments            { NewExprNode* node = new NewExprNode(GLOBAL_DATA, $2.m_node, $3.m_node);
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @3.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $2.m_features | $3.m_features, $2.m_numConstants + $3.m_numConstants);
                                        }
;

MemberExprNoBF:
    PrimaryExprNoBrace
  | MemberExprNoBF LBRACKET Expr RBRACKET         { BracketAccessorNode* node = new BracketAccessorNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @4.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); 
                                        }
  | MemberExprNoBF DOT IDENT            { DotAccessorNode* node = new DotAccessorNode(GLOBAL_DATA, $1.m_node, *$3);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @3.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features, $1.m_numConstants);
                                        }
  | NEW MemberExpr Arguments            { NewExprNode* node = new NewExprNode(GLOBAL_DATA, $2.m_node, $3.m_node);
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @3.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $2.m_features | $3.m_features, $2.m_numConstants + $3.m_numConstants);
                                        }
;

NewExpr:
    MemberExpr
  | NEW NewExpr                         { NewExprNode* node = new NewExprNode(GLOBAL_DATA, $2.m_node);
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @2.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $2.m_features, $2.m_numConstants); 
                                        }
;

NewExprNoBF:
    MemberExprNoBF
  | NEW NewExpr                         { NewExprNode* node = new NewExprNode(GLOBAL_DATA, $2.m_node);
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @2.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $2.m_features, $2.m_numConstants);
                                        }
;

CallExpr:
    MemberExpr Arguments                { $$ = makeFunctionCallNode(GLOBAL_DATA, $1, $2, @1.first_column, @1.last_column, @2.last_column); }
  | CallExpr Arguments                  { $$ = makeFunctionCallNode(GLOBAL_DATA, $1, $2, @1.first_column, @1.last_column, @2.last_column); }
  | CallExpr LBRACKET Expr RBRACKET               { BracketAccessorNode* node = new BracketAccessorNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @4.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); 
                                        }
  | CallExpr DOT IDENT                  { DotAccessorNode* node = new DotAccessorNode(GLOBAL_DATA, $1.m_node, *$3);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @3.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features, $1.m_numConstants); }
;

CallExprNoBF:
    MemberExprNoBF Arguments            { $$ = makeFunctionCallNode(GLOBAL_DATA, $1, $2, @1.first_column, @1.last_column, @2.last_column); }
  | CallExprNoBF Arguments              { $$ = makeFunctionCallNode(GLOBAL_DATA, $1, $2, @1.first_column, @1.last_column, @2.last_column); }
  | CallExprNoBF LBRACKET Expr RBRACKET           { BracketAccessorNode* node = new BracketAccessorNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @4.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); 
                                        }
  | CallExprNoBF DOT IDENT              { DotAccessorNode* node = new DotAccessorNode(GLOBAL_DATA, $1.m_node, *$3);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @3.last_column);
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features, $1.m_numConstants); 
                                        }
;

Arguments:
    LPAREN RPAREN                             { $$ = createNodeInfo<ArgumentsNode*>(new ArgumentsNode(GLOBAL_DATA), 0, 0); }
  | LPAREN ArgumentList RPAREN                { $$ = createNodeInfo<ArgumentsNode*>(new ArgumentsNode(GLOBAL_DATA, $2.m_node.head), $2.m_features, $2.m_numConstants); }
;

ArgumentList:
    AssignmentExpr                      { $$.m_node.head = new ArgumentListNode(GLOBAL_DATA, $1.m_node);
                                          $$.m_node.tail = $$.m_node.head;
                                          $$.m_features = $1.m_features;
                                          $$.m_numConstants = $1.m_numConstants; }
  | ArgumentList COMMA AssignmentExpr     { $$.m_node.head = $1.m_node.head;
                                          $$.m_node.tail = new ArgumentListNode(GLOBAL_DATA, $3.m_node, $1.m_node.tail);
                                          $$.m_features = $1.m_features | $3.m_features;
                                          $$.m_numConstants = $1.m_numConstants + $3.m_numConstants; }
;

LeftHandSideExpr:
    NewExpr
  | CallExpr
;

LeftHandSideExprNoBF:
    NewExprNoBF
  | CallExprNoBF
;

PostfixExpr:
    LeftHandSideExpr
  | LeftHandSideExpr PLUSPLUS           { $$ = createNodeInfo<ExpressionNode*>(makePostfixNode(GLOBAL_DATA, $1.m_node, op_plusplus, @1.first_column, @1.last_column, @2.last_column), $1.m_features | AssignFeature, $1.m_numConstants); }
  | LeftHandSideExpr MINUSMINUS         { $$ = createNodeInfo<ExpressionNode*>(makePostfixNode(GLOBAL_DATA, $1.m_node, op_minusminus, @1.first_column, @1.last_column, @2.last_column), $1.m_features | AssignFeature, $1.m_numConstants); }
;

PostfixExprNoBF:
    LeftHandSideExprNoBF
  | LeftHandSideExprNoBF PLUSPLUS       { $$ = createNodeInfo<ExpressionNode*>(makePostfixNode(GLOBAL_DATA, $1.m_node, op_plusplus, @1.first_column, @1.last_column, @2.last_column), $1.m_features | AssignFeature, $1.m_numConstants); }
  | LeftHandSideExprNoBF MINUSMINUS     { $$ = createNodeInfo<ExpressionNode*>(makePostfixNode(GLOBAL_DATA, $1.m_node, op_minusminus, @1.first_column, @1.last_column, @2.last_column), $1.m_features | AssignFeature, $1.m_numConstants); }
;

UnaryExprCommon:
    DELETETOKEN UnaryExpr               { $$ = createNodeInfo<ExpressionNode*>(makeDeleteNode(GLOBAL_DATA, $2.m_node, @1.first_column, @2.last_column, @2.last_column), $2.m_features, $2.m_numConstants); }
  | VOIDTOKEN UnaryExpr                 { $$ = createNodeInfo<ExpressionNode*>(new VoidNode(GLOBAL_DATA, $2.m_node), $2.m_features, $2.m_numConstants + 1); }
  | TYPEOF UnaryExpr                    { $$ = createNodeInfo<ExpressionNode*>(makeTypeOfNode(GLOBAL_DATA, $2.m_node), $2.m_features, $2.m_numConstants); }
  | PLUSPLUS UnaryExpr                  { $$ = createNodeInfo<ExpressionNode*>(makePrefixNode(GLOBAL_DATA, $2.m_node, op_plusplus, @1.first_column, @2.first_column + 1, @2.last_column), $2.m_features | AssignFeature, $2.m_numConstants); }
  | AUTOPLUSPLUS UnaryExpr              { $$ = createNodeInfo<ExpressionNode*>(makePrefixNode(GLOBAL_DATA, $2.m_node, op_plusplus, @1.first_column, @2.first_column + 1, @2.last_column), $2.m_features | AssignFeature, $2.m_numConstants); }
  | MINUSMINUS UnaryExpr                { $$ = createNodeInfo<ExpressionNode*>(makePrefixNode(GLOBAL_DATA, $2.m_node, op_minusminus, @1.first_column, @2.first_column + 1, @2.last_column), $2.m_features | AssignFeature, $2.m_numConstants); }
  | AUTOMINUSMINUS UnaryExpr            { $$ = createNodeInfo<ExpressionNode*>(makePrefixNode(GLOBAL_DATA, $2.m_node, op_minusminus, @1.first_column, @2.first_column + 1, @2.last_column), $2.m_features | AssignFeature, $2.m_numConstants); }
  | PLUS UnaryExpr                      { $$ = createNodeInfo<ExpressionNode*>(new UnaryPlusNode(GLOBAL_DATA, $2.m_node), $2.m_features, $2.m_numConstants); }
  | MINUS UnaryExpr                     { $$ = createNodeInfo<ExpressionNode*>(makeNegateNode(GLOBAL_DATA, $2.m_node), $2.m_features, $2.m_numConstants); }
  | BITINV UnaryExpr                    { $$ = createNodeInfo<ExpressionNode*>(makeBitwiseNotNode(GLOBAL_DATA, $2.m_node), $2.m_features, $2.m_numConstants); }
  | NOT UnaryExpr                       { $$ = createNodeInfo<ExpressionNode*>(new LogicalNotNode(GLOBAL_DATA, $2.m_node), $2.m_features, $2.m_numConstants); }

UnaryExpr:
    PostfixExpr
  | UnaryExprCommon
;

UnaryExprNoBF:
    PostfixExprNoBF
  | UnaryExprCommon
;

MultiplicativeExpr:
    UnaryExpr
  | MultiplicativeExpr MUL UnaryExpr    { $$ = createNodeInfo<ExpressionNode*>(makeMultNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | MultiplicativeExpr DIV UnaryExpr    { $$ = createNodeInfo<ExpressionNode*>(makeDivNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | MultiplicativeExpr MOD UnaryExpr    { $$ = createNodeInfo<ExpressionNode*>(new ModNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

MultiplicativeExprNoBF:
    UnaryExprNoBF
  | MultiplicativeExprNoBF MUL UnaryExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(makeMultNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | MultiplicativeExprNoBF DIV UnaryExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(makeDivNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | MultiplicativeExprNoBF MOD UnaryExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(new ModNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

AdditiveExpr:
    MultiplicativeExpr
  | AdditiveExpr PLUS MultiplicativeExpr { $$ = createNodeInfo<ExpressionNode*>(makeAddNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | AdditiveExpr MINUS MultiplicativeExpr { $$ = createNodeInfo<ExpressionNode*>(makeSubNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

AdditiveExprNoBF:
    MultiplicativeExprNoBF
  | AdditiveExprNoBF PLUS MultiplicativeExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(makeAddNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | AdditiveExprNoBF MINUS MultiplicativeExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(makeSubNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

ShiftExpr:
    AdditiveExpr
  | ShiftExpr LSHIFT AdditiveExpr       { $$ = createNodeInfo<ExpressionNode*>(makeLeftShiftNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | ShiftExpr RSHIFT AdditiveExpr       { $$ = createNodeInfo<ExpressionNode*>(makeRightShiftNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | ShiftExpr URSHIFT AdditiveExpr      { $$ = createNodeInfo<ExpressionNode*>(new UnsignedRightShiftNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

ShiftExprNoBF:
    AdditiveExprNoBF
  | ShiftExprNoBF LSHIFT AdditiveExpr   { $$ = createNodeInfo<ExpressionNode*>(makeLeftShiftNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | ShiftExprNoBF RSHIFT AdditiveExpr   { $$ = createNodeInfo<ExpressionNode*>(makeRightShiftNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | ShiftExprNoBF URSHIFT AdditiveExpr  { $$ = createNodeInfo<ExpressionNode*>(new UnsignedRightShiftNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

RelationalExpr:
    ShiftExpr
  | RelationalExpr LT ShiftExpr        { $$ = createNodeInfo<ExpressionNode*>(new LessNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExpr GT ShiftExpr        { $$ = createNodeInfo<ExpressionNode*>(new GreaterNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExpr LE ShiftExpr         { $$ = createNodeInfo<ExpressionNode*>(new LessEqNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExpr GE ShiftExpr         { $$ = createNodeInfo<ExpressionNode*>(new GreaterEqNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExpr INSTANCEOF ShiftExpr { InstanceOfNode* node = new InstanceOfNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature);
                                          setExceptionLocation(node, @1.first_column, @3.first_column, @3.last_column);  
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExpr INTOKEN ShiftExpr    { InNode* node = new InNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature);
                                          setExceptionLocation(node, @1.first_column, @3.first_column, @3.last_column);  
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

RelationalExprNoIn:
    ShiftExpr
  | RelationalExprNoIn LT ShiftExpr    { $$ = createNodeInfo<ExpressionNode*>(new LessNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExprNoIn GT ShiftExpr    { $$ = createNodeInfo<ExpressionNode*>(new GreaterNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExprNoIn LE ShiftExpr     { $$ = createNodeInfo<ExpressionNode*>(new LessEqNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExprNoIn GE ShiftExpr     { $$ = createNodeInfo<ExpressionNode*>(new GreaterEqNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExprNoIn INSTANCEOF ShiftExpr
                                        { InstanceOfNode* node = new InstanceOfNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature);
                                          setExceptionLocation(node, @1.first_column, @3.first_column, @3.last_column);  
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

RelationalExprNoBF:
    ShiftExprNoBF
  | RelationalExprNoBF LT ShiftExpr    { $$ = createNodeInfo<ExpressionNode*>(new LessNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExprNoBF GT ShiftExpr    { $$ = createNodeInfo<ExpressionNode*>(new GreaterNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExprNoBF LE ShiftExpr     { $$ = createNodeInfo<ExpressionNode*>(new LessEqNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExprNoBF GE ShiftExpr     { $$ = createNodeInfo<ExpressionNode*>(new GreaterEqNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExprNoBF INSTANCEOF ShiftExpr
                                        { InstanceOfNode* node = new InstanceOfNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature);
                                          setExceptionLocation(node, @1.first_column, @3.first_column, @3.last_column);  
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | RelationalExprNoBF INTOKEN ShiftExpr 
                                        { InNode* node = new InNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature);
                                          setExceptionLocation(node, @1.first_column, @3.first_column, @3.last_column);  
                                          $$ = createNodeInfo<ExpressionNode*>(node, $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

EqualityExpr:
    RelationalExpr
  | EqualityExpr EQEQ RelationalExpr    { $$ = createNodeInfo<ExpressionNode*>(new EqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | EqualityExpr NE RelationalExpr      { $$ = createNodeInfo<ExpressionNode*>(new NotEqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | EqualityExpr STREQ RelationalExpr   { $$ = createNodeInfo<ExpressionNode*>(new StrictEqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | EqualityExpr STRNEQ RelationalExpr  { $$ = createNodeInfo<ExpressionNode*>(new NotStrictEqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

EqualityExprNoIn:
    RelationalExprNoIn
  | EqualityExprNoIn EQEQ RelationalExprNoIn
                                        { $$ = createNodeInfo<ExpressionNode*>(new EqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | EqualityExprNoIn NE RelationalExprNoIn
                                        { $$ = createNodeInfo<ExpressionNode*>(new NotEqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | EqualityExprNoIn STREQ RelationalExprNoIn
                                        { $$ = createNodeInfo<ExpressionNode*>(new StrictEqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | EqualityExprNoIn STRNEQ RelationalExprNoIn
                                        { $$ = createNodeInfo<ExpressionNode*>(new NotStrictEqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

EqualityExprNoBF:
    RelationalExprNoBF
  | EqualityExprNoBF EQEQ RelationalExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(new EqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | EqualityExprNoBF NE RelationalExpr  { $$ = createNodeInfo<ExpressionNode*>(new NotEqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | EqualityExprNoBF STREQ RelationalExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(new StrictEqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
  | EqualityExprNoBF STRNEQ RelationalExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(new NotStrictEqualNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

BitwiseANDExpr:
    EqualityExpr
  | BitwiseANDExpr BITAND EqualityExpr     { $$ = createNodeInfo<ExpressionNode*>(new BitAndNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

BitwiseANDExprNoIn:
    EqualityExprNoIn
  | BitwiseANDExprNoIn BITAND EqualityExprNoIn
                                        { $$ = createNodeInfo<ExpressionNode*>(new BitAndNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

BitwiseANDExprNoBF:
    EqualityExprNoBF
  | BitwiseANDExprNoBF BITAND EqualityExpr { $$ = createNodeInfo<ExpressionNode*>(new BitAndNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

BitwiseXORExpr:
    BitwiseANDExpr
  | BitwiseXORExpr BITXOR BitwiseANDExpr   { $$ = createNodeInfo<ExpressionNode*>(new BitXOrNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

BitwiseXORExprNoIn:
    BitwiseANDExprNoIn
  | BitwiseXORExprNoIn BITXOR BitwiseANDExprNoIn
                                        { $$ = createNodeInfo<ExpressionNode*>(new BitXOrNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

BitwiseXORExprNoBF:
    BitwiseANDExprNoBF
  | BitwiseXORExprNoBF BITXOR BitwiseANDExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(new BitXOrNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

BitwiseORExpr:
    BitwiseXORExpr
  | BitwiseORExpr BITOR BitwiseXORExpr    { $$ = createNodeInfo<ExpressionNode*>(new BitOrNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

BitwiseORExprNoIn:
    BitwiseXORExprNoIn
  | BitwiseORExprNoIn BITOR BitwiseXORExprNoIn
                                        { $$ = createNodeInfo<ExpressionNode*>(new BitOrNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

BitwiseORExprNoBF:
    BitwiseXORExprNoBF
  | BitwiseORExprNoBF BITOR BitwiseXORExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(new BitOrNode(GLOBAL_DATA, $1.m_node, $3.m_node, $3.m_features & AssignFeature), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

LogicalANDExpr:
    BitwiseORExpr
  | LogicalANDExpr AND BitwiseORExpr    { $$ = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, $1.m_node, $3.m_node, op_and), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

LogicalANDExprNoIn:
    BitwiseORExprNoIn
  | LogicalANDExprNoIn AND BitwiseORExprNoIn
                                        { $$ = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, $1.m_node, $3.m_node, op_and), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

LogicalANDExprNoBF:
    BitwiseORExprNoBF
  | LogicalANDExprNoBF AND BitwiseORExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, $1.m_node, $3.m_node, op_and), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

LogicalORExpr:
    LogicalANDExpr
  | LogicalORExpr OR LogicalANDExpr     { $$ = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, $1.m_node, $3.m_node, op_or), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

LogicalORExprNoIn:
    LogicalANDExprNoIn
  | LogicalORExprNoIn OR LogicalANDExprNoIn
                                        { $$ = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, $1.m_node, $3.m_node, op_or), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

LogicalORExprNoBF:
    LogicalANDExprNoBF
  | LogicalORExprNoBF OR LogicalANDExpr { $$ = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, $1.m_node, $3.m_node, op_or), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

ConditionalExpr:
    LogicalORExpr
  | LogicalORExpr QUESTIONMARK AssignmentExpr COLON AssignmentExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(new ConditionalNode(GLOBAL_DATA, $1.m_node, $3.m_node, $5.m_node), $1.m_features | $3.m_features | $5.m_features, $1.m_numConstants + $3.m_numConstants + $5.m_numConstants); }
;

ConditionalExprNoIn:
    LogicalORExprNoIn
  | LogicalORExprNoIn QUESTIONMARK AssignmentExprNoIn COLON AssignmentExprNoIn
                                        { $$ = createNodeInfo<ExpressionNode*>(new ConditionalNode(GLOBAL_DATA, $1.m_node, $3.m_node, $5.m_node), $1.m_features | $3.m_features | $5.m_features, $1.m_numConstants + $3.m_numConstants + $5.m_numConstants); }
;

ConditionalExprNoBF:
    LogicalORExprNoBF
  | LogicalORExprNoBF QUESTIONMARK AssignmentExpr COLON AssignmentExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(new ConditionalNode(GLOBAL_DATA, $1.m_node, $3.m_node, $5.m_node), $1.m_features | $3.m_features | $5.m_features, $1.m_numConstants + $3.m_numConstants + $5.m_numConstants); }
;

AssignmentExpr:
    ConditionalExpr
  | LeftHandSideExpr AssignmentOperator AssignmentExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(makeAssignNode(GLOBAL_DATA, $1.m_node, $2, $3.m_node, $1.m_features & AssignFeature, $3.m_features & AssignFeature, 
                                                                                                     @1.first_column, @2.first_column + 1, @3.last_column), $1.m_features | $3.m_features | AssignFeature, $1.m_numConstants + $3.m_numConstants); 
                                        }
;

AssignmentExprNoIn:
    ConditionalExprNoIn
  | LeftHandSideExpr AssignmentOperator AssignmentExprNoIn
                                        { $$ = createNodeInfo<ExpressionNode*>(makeAssignNode(GLOBAL_DATA, $1.m_node, $2, $3.m_node, $1.m_features & AssignFeature, $3.m_features & AssignFeature, 
                                                                                                     @1.first_column, @2.first_column + 1, @3.last_column), $1.m_features | $3.m_features | AssignFeature, $1.m_numConstants + $3.m_numConstants);
                                        }
;

AssignmentExprNoBF:
    ConditionalExprNoBF
  | LeftHandSideExprNoBF AssignmentOperator AssignmentExpr
                                        { $$ = createNodeInfo<ExpressionNode*>(makeAssignNode(GLOBAL_DATA, $1.m_node, $2, $3.m_node, $1.m_features & AssignFeature, $3.m_features & AssignFeature,
                                                                                                     @1.first_column, @2.first_column + 1, @3.last_column), $1.m_features | $3.m_features | AssignFeature, $1.m_numConstants + $3.m_numConstants); 
                                        }
;

AssignmentOperator:
    EQUAL                                 { $$ = op_eq; }
  | PLUSEQUAL                           { $$ = op_pluseq; }
  | MINUSEQUAL                          { $$ = op_minuseq; }
  | MULTEQUAL                           { $$ = op_multeq; }
  | DIVEQUAL                            { $$ = op_diveq; }
  | LSHIFTEQUAL                         { $$ = op_lshift; }
  | RSHIFTEQUAL                         { $$ = op_rshift; }
  | URSHIFTEQUAL                        { $$ = op_urshift; }
  | ANDEQUAL                            { $$ = op_andeq; }
  | XOREQUAL                            { $$ = op_xoreq; }
  | OREQUAL                             { $$ = op_oreq; }
  | MODEQUAL                            { $$ = op_modeq; }
;

Expr:
    AssignmentExpr
  | Expr COMMA AssignmentExpr             { $$ = createNodeInfo<ExpressionNode*>(combineCommaNodes(GLOBAL_DATA, $1.m_node, $3.m_node), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

ExprNoIn:
    AssignmentExprNoIn
  | ExprNoIn COMMA AssignmentExprNoIn     { $$ = createNodeInfo<ExpressionNode*>(combineCommaNodes(GLOBAL_DATA, $1.m_node, $3.m_node), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

ExprNoBF:
    AssignmentExprNoBF
  | ExprNoBF COMMA AssignmentExpr         { $$ = createNodeInfo<ExpressionNode*>(combineCommaNodes(GLOBAL_DATA, $1.m_node, $3.m_node), $1.m_features | $3.m_features, $1.m_numConstants + $3.m_numConstants); }
;

Statement:
    Block
  | VariableStatement
  | ConstStatement
  | FunctionDeclaration
  | EmptyStatement
  | ExprStatement
  | IfStatement
  | IterationStatement
  | ContinueStatement
  | BreakStatement
  | ReturnStatement
  | WithStatement
  | SwitchStatement
  | LabelledStatement
  | ThrowStatement
  | TryStatement
  | DebuggerStatement
;

Block:
    OPENBRACE CLOSEBRACE                { $$ = createNodeDeclarationInfo<StatementNode*>(new BlockNode(GLOBAL_DATA, 0), 0, 0, 0, 0);
                                          setStatementLocation($$.m_node, @1, @2); }
  | OPENBRACE SourceElements CLOSEBRACE { $$ = createNodeDeclarationInfo<StatementNode*>(new BlockNode(GLOBAL_DATA, $2.m_node), $2.m_varDeclarations, $2.m_funcDeclarations, $2.m_features, $2.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @3); }
;

VariableStatement:
    VAR VariableDeclarationList SEMICOLON     { $$ = createNodeDeclarationInfo<StatementNode*>(makeVarStatementNode(GLOBAL_DATA, $2.m_node), $2.m_varDeclarations, $2.m_funcDeclarations, $2.m_features, $2.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @3); }
  | VAR VariableDeclarationList error   { $$ = createNodeDeclarationInfo<StatementNode*>(makeVarStatementNode(GLOBAL_DATA, $2.m_node), $2.m_varDeclarations, $2.m_funcDeclarations, $2.m_features, $2.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @2);
                                          AUTO_SEMICOLON; }
;

VariableDeclarationList:
    IDENT                               { ResolveNode* node = new ResolveNode(GLOBAL_DATA, *$1, @1.first_column);
										  $$.m_node = node;
                                          $$.m_varDeclarations = new ParserArenaData<DeclarationStacks::VarStack>;
                                          appendToVarDeclarationList(GLOBAL_DATA, $$.m_varDeclarations, *$1, 0);
                                          $$.m_funcDeclarations = 0;
                                          $$.m_features = (*$1 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0;
                                          $$.m_numConstants = 0;
                                        }
  | IDENT Initializer                   { AssignResolveNode* node = new AssignResolveNode(GLOBAL_DATA, *$1, $2.m_node, $2.m_features & AssignFeature);
                                          setExceptionLocation(node, @1.first_column, @2.first_column + 1, @2.last_column);
                                          $$.m_node = node;
                                          $$.m_varDeclarations = new ParserArenaData<DeclarationStacks::VarStack>;
                                          appendToVarDeclarationList(GLOBAL_DATA, $$.m_varDeclarations, *$1, DeclarationStacks::HasInitializer);
                                          $$.m_funcDeclarations = 0;
                                          $$.m_features = ((*$1 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | $2.m_features;
                                          $$.m_numConstants = $2.m_numConstants;
                                        }
  | VariableDeclarationList COMMA IDENT
                                        { /* eddid add ResolveNode node to fix "var a,b,c=2; b lost" issue */
										  ResolveNode* node = new ResolveNode(GLOBAL_DATA, *$3, @3.first_column);
                                          $$.m_node = combineCommaNodes(GLOBAL_DATA, $1.m_node, node);
                                          $$.m_varDeclarations = $1.m_varDeclarations;
                                          appendToVarDeclarationList(GLOBAL_DATA, $$.m_varDeclarations, *$3, 0);
                                          $$.m_funcDeclarations = 0;
                                          $$.m_features = $1.m_features | ((*$3 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0);
                                          $$.m_numConstants = $1.m_numConstants;
                                        }
  | VariableDeclarationList COMMA IDENT Initializer
                                        { AssignResolveNode* node = new AssignResolveNode(GLOBAL_DATA, *$3, $4.m_node, $4.m_features & AssignFeature);
                                          setExceptionLocation(node, @3.first_column, @4.first_column + 1, @4.last_column);
                                          $$.m_node = combineCommaNodes(GLOBAL_DATA, $1.m_node, node);
                                          $$.m_varDeclarations = $1.m_varDeclarations;
                                          appendToVarDeclarationList(GLOBAL_DATA, $$.m_varDeclarations, *$3, DeclarationStacks::HasInitializer);
                                          $$.m_funcDeclarations = 0;
                                          $$.m_features = $1.m_features | ((*$3 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | $4.m_features;
                                          $$.m_numConstants = $1.m_numConstants + $4.m_numConstants;
                                        }
;

VariableDeclarationListNoIn:
    IDENT                               { $$.m_node = 0;
                                          $$.m_varDeclarations = new ParserArenaData<DeclarationStacks::VarStack>;
                                          appendToVarDeclarationList(GLOBAL_DATA, $$.m_varDeclarations, *$1, 0);
                                          $$.m_funcDeclarations = 0;
                                          $$.m_features = (*$1 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0;
                                          $$.m_numConstants = 0;
                                        }
  | IDENT InitializerNoIn               { AssignResolveNode* node = new AssignResolveNode(GLOBAL_DATA, *$1, $2.m_node, $2.m_features & AssignFeature);
                                          setExceptionLocation(node, @1.first_column, @2.first_column + 1, @2.last_column);
                                          $$.m_node = node;
                                          $$.m_varDeclarations = new ParserArenaData<DeclarationStacks::VarStack>;
                                          appendToVarDeclarationList(GLOBAL_DATA, $$.m_varDeclarations, *$1, DeclarationStacks::HasInitializer);
                                          $$.m_funcDeclarations = 0;
                                          $$.m_features = ((*$1 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | $2.m_features;
                                          $$.m_numConstants = $2.m_numConstants;
                                        }
  | VariableDeclarationListNoIn COMMA IDENT
                                        { $$.m_node = $1.m_node;
                                          $$.m_varDeclarations = $1.m_varDeclarations;
                                          appendToVarDeclarationList(GLOBAL_DATA, $$.m_varDeclarations, *$3, 0);
                                          $$.m_funcDeclarations = 0;
                                          $$.m_features = $1.m_features | ((*$3 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0);
                                          $$.m_numConstants = $1.m_numConstants;
                                        }
  | VariableDeclarationListNoIn COMMA IDENT InitializerNoIn
                                        { AssignResolveNode* node = new AssignResolveNode(GLOBAL_DATA, *$3, $4.m_node, $4.m_features & AssignFeature);
                                          setExceptionLocation(node, @3.first_column, @4.first_column + 1, @4.last_column);
                                          $$.m_node = combineCommaNodes(GLOBAL_DATA, $1.m_node, node);
                                          $$.m_varDeclarations = $1.m_varDeclarations;
                                          appendToVarDeclarationList(GLOBAL_DATA, $$.m_varDeclarations, *$3, DeclarationStacks::HasInitializer);
                                          $$.m_funcDeclarations = 0;
                                          $$.m_features = $1.m_features | ((*$3 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | $4.m_features;
                                          $$.m_numConstants = $1.m_numConstants + $4.m_numConstants;
                                        }
;

ConstStatement:
    CONSTTOKEN ConstDeclarationList SEMICOLON { $$ = createNodeDeclarationInfo<StatementNode*>(new ConstStatementNode(GLOBAL_DATA, $2.m_node.head), $2.m_varDeclarations, $2.m_funcDeclarations, $2.m_features, $2.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @3); }
  | CONSTTOKEN ConstDeclarationList error
                                        { $$ = createNodeDeclarationInfo<StatementNode*>(new ConstStatementNode(GLOBAL_DATA, $2.m_node.head), $2.m_varDeclarations, $2.m_funcDeclarations, $2.m_features, $2.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @2); AUTO_SEMICOLON; }
;

ConstDeclarationList:
    ConstDeclaration                    { $$.m_node.head = $1.m_node;
                                          $$.m_node.tail = $$.m_node.head;
                                          $$.m_varDeclarations = new ParserArenaData<DeclarationStacks::VarStack>;
                                          appendToVarDeclarationList(GLOBAL_DATA, $$.m_varDeclarations, $1.m_node);
                                          $$.m_funcDeclarations = 0; 
                                          $$.m_features = $1.m_features;
                                          $$.m_numConstants = $1.m_numConstants;
    }
  | ConstDeclarationList COMMA ConstDeclaration
                                        { $$.m_node.head = $1.m_node.head;
                                          $1.m_node.tail->setNext($3.m_node);
                                          $$.m_node.tail = $3.m_node;
                                          $$.m_varDeclarations = $1.m_varDeclarations;
                                          appendToVarDeclarationList(GLOBAL_DATA, $$.m_varDeclarations, $3.m_node);
                                          $$.m_funcDeclarations = 0;
                                          $$.m_features = $1.m_features | $3.m_features;
                                          $$.m_numConstants = $1.m_numConstants + $3.m_numConstants; }
;

ConstDeclaration:
    IDENT                               { $$ = createNodeInfo<ConstDeclNode*>(new ConstDeclNode(GLOBAL_DATA, *$1, 0), (*$1 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0, 0); }
  | IDENT Initializer                   { $$ = createNodeInfo<ConstDeclNode*>(new ConstDeclNode(GLOBAL_DATA, *$1, $2.m_node), ((*$1 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | $2.m_features, $2.m_numConstants); }
;

Initializer:
    EQUAL AssignmentExpr                  { $$ = $2; }
;

InitializerNoIn:
    EQUAL AssignmentExprNoIn              { $$ = $2; }
;

EmptyStatement:
    SEMICOLON                                 { $$ = createNodeDeclarationInfo<StatementNode*>(new EmptyStatementNode(GLOBAL_DATA), 0, 0, 0, 0); }
;

ExprStatement:
    ExprNoBF SEMICOLON                        { $$ = createNodeDeclarationInfo<StatementNode*>(new ExprStatementNode(GLOBAL_DATA, $1.m_node), 0, 0, $1.m_features, $1.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @2); }
  | ExprNoBF error                      { $$ = createNodeDeclarationInfo<StatementNode*>(new ExprStatementNode(GLOBAL_DATA, $1.m_node), 0, 0, $1.m_features, $1.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @1); AUTO_SEMICOLON; }
;

IfStatement:
    IF LPAREN Expr RPAREN Statement %prec IF_WITHOUT_ELSE
                                        { $$ = createNodeDeclarationInfo<StatementNode*>(new IfNode(GLOBAL_DATA, $3.m_node, $5.m_node), $5.m_varDeclarations, $5.m_funcDeclarations, $3.m_features | $5.m_features, $3.m_numConstants + $5.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @4); }
  | IF LPAREN Expr RPAREN Statement ELSE Statement
                                        { $$ = createNodeDeclarationInfo<StatementNode*>(new IfElseNode(GLOBAL_DATA, $3.m_node, $5.m_node, $7.m_node), 
                                                                                         mergeDeclarationLists($5.m_varDeclarations, $7.m_varDeclarations),
                                                                                         mergeDeclarationLists($5.m_funcDeclarations, $7.m_funcDeclarations),
                                                                                         $3.m_features | $5.m_features | $7.m_features,
                                                                                         $3.m_numConstants + $5.m_numConstants + $7.m_numConstants); 
                                          setStatementLocation($$.m_node, @1, @4); }
;

IterationStatement:
    DO Statement WHILE LPAREN Expr RPAREN SEMICOLON    { $$ = createNodeDeclarationInfo<StatementNode*>(new DoWhileNode(GLOBAL_DATA, $2.m_node, $5.m_node), $2.m_varDeclarations, $2.m_funcDeclarations, $2.m_features | $5.m_features, $2.m_numConstants + $5.m_numConstants);
                                             setStatementLocation($$.m_node, @1, @3); }
  | DO Statement WHILE LPAREN Expr RPAREN error  { $$ = createNodeDeclarationInfo<StatementNode*>(new DoWhileNode(GLOBAL_DATA, $2.m_node, $5.m_node), $2.m_varDeclarations, $2.m_funcDeclarations, $2.m_features | $5.m_features, $2.m_numConstants + $5.m_numConstants);
                                             setStatementLocation($$.m_node, @1, @3); } // Always performs automatic semicolon insertion.
  | WHILE LPAREN Expr RPAREN Statement        { $$ = createNodeDeclarationInfo<StatementNode*>(new WhileNode(GLOBAL_DATA, $3.m_node, $5.m_node), $5.m_varDeclarations, $5.m_funcDeclarations, $3.m_features | $5.m_features, $3.m_numConstants + $5.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @4); }
  | FOR LPAREN ExprNoInOpt SEMICOLON ExprOpt SEMICOLON ExprOpt RPAREN Statement
                                        { $$ = createNodeDeclarationInfo<StatementNode*>(new ForNode(GLOBAL_DATA, $3.m_node, $5.m_node, $7.m_node, $9.m_node, false), $9.m_varDeclarations, $9.m_funcDeclarations, 
                                                                                         $3.m_features | $5.m_features | $7.m_features | $9.m_features,
                                                                                         $3.m_numConstants + $5.m_numConstants + $7.m_numConstants + $9.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @8); 
                                        }
  | FOR LPAREN VAR VariableDeclarationListNoIn SEMICOLON ExprOpt SEMICOLON ExprOpt RPAREN Statement
                                        { $$ = createNodeDeclarationInfo<StatementNode*>(new ForNode(GLOBAL_DATA, $4.m_node, $6.m_node, $8.m_node, $10.m_node, true),
                                                                                         mergeDeclarationLists($4.m_varDeclarations, $10.m_varDeclarations),
                                                                                         mergeDeclarationLists($4.m_funcDeclarations, $10.m_funcDeclarations),
                                                                                         $4.m_features | $6.m_features | $8.m_features | $10.m_features,
                                                                                         $4.m_numConstants + $6.m_numConstants + $8.m_numConstants + $10.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @9); }
  | FOR LPAREN LeftHandSideExpr INTOKEN Expr RPAREN Statement
                                        {
                                            ForInNode* node = new ForInNode(GLOBAL_DATA, $3.m_node, $5.m_node, $7.m_node);
                                            setExceptionLocation(node, @3.first_column, @3.last_column, @5.last_column);
                                            $$ = createNodeDeclarationInfo<StatementNode*>(node, $7.m_varDeclarations, $7.m_funcDeclarations,
                                                                                           $3.m_features | $5.m_features | $7.m_features,
                                                                                           $3.m_numConstants + $5.m_numConstants + $7.m_numConstants);
                                            setStatementLocation($$.m_node, @1, @6);
                                        }
  | FOR LPAREN VAR IDENT INTOKEN Expr RPAREN Statement
                                        { ForInNode *forIn = new ForInNode(GLOBAL_DATA, *$4, 0, $6.m_node, $8.m_node, @5.first_column, @5.first_column - @4.first_column, @6.last_column - @5.first_column);
                                          setExceptionLocation(forIn, @4.first_column, @5.first_column + 1, @6.last_column);
                                          appendToVarDeclarationList(GLOBAL_DATA, $8.m_varDeclarations, *$4, DeclarationStacks::HasInitializer);
                                          $$ = createNodeDeclarationInfo<StatementNode*>(forIn, $8.m_varDeclarations, $8.m_funcDeclarations, ((*$4 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | $6.m_features | $8.m_features, $6.m_numConstants + $8.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @7); }
  | FOR LPAREN VAR IDENT InitializerNoIn INTOKEN Expr RPAREN Statement
                                        { ForInNode *forIn = new ForInNode(GLOBAL_DATA, *$4, $5.m_node, $7.m_node, $9.m_node, @5.first_column, @5.first_column - @4.first_column, @5.last_column - @5.first_column);
                                          setExceptionLocation(forIn, @4.first_column, @6.first_column + 1, @7.last_column);
                                          appendToVarDeclarationList(GLOBAL_DATA, $9.m_varDeclarations, *$4, DeclarationStacks::HasInitializer);
                                          $$ = createNodeDeclarationInfo<StatementNode*>(forIn, $9.m_varDeclarations, $9.m_funcDeclarations,
                                                                                         ((*$4 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | $5.m_features | $7.m_features | $9.m_features,
                                                                                         $5.m_numConstants + $7.m_numConstants + $9.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @8); }
;

ExprOpt:
    /* nothing */                       { $$ = createNodeInfo<ExpressionNode*>(0, 0, 0); }
  | Expr
;

ExprNoInOpt:
    /* nothing */                       { $$ = createNodeInfo<ExpressionNode*>(0, 0, 0); }
  | ExprNoIn
;

ContinueStatement:
    CONTINUE SEMICOLON                        { ContinueNode* node = new ContinueNode(GLOBAL_DATA);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @1.last_column); 
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0);
                                          setStatementLocation($$.m_node, @1, @2); }
  | CONTINUE error                      { ContinueNode* node = new ContinueNode(GLOBAL_DATA);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @1.last_column); 
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0);
                                          setStatementLocation($$.m_node, @1, @1); AUTO_SEMICOLON; }
  | CONTINUE IDENT SEMICOLON                  { ContinueNode* node = new ContinueNode(GLOBAL_DATA, *$2);
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @2.last_column); 
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0);
                                          setStatementLocation($$.m_node, @1, @3); }
  | CONTINUE IDENT error                { ContinueNode* node = new ContinueNode(GLOBAL_DATA, *$2);
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @2.last_column); 
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0);
                                          setStatementLocation($$.m_node, @1, @2); AUTO_SEMICOLON; }
;

BreakStatement:
    BREAK SEMICOLON                           { BreakNode* node = new BreakNode(GLOBAL_DATA);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @1.last_column);
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0); setStatementLocation($$.m_node, @1, @2); }
  | BREAK error                         { BreakNode* node = new BreakNode(GLOBAL_DATA);
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @1.last_column);
                                          $$ = createNodeDeclarationInfo<StatementNode*>(new BreakNode(GLOBAL_DATA), 0, 0, 0, 0); setStatementLocation($$.m_node, @1, @1); AUTO_SEMICOLON; }
  | BREAK IDENT SEMICOLON                     { BreakNode* node = new BreakNode(GLOBAL_DATA, *$2);
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @2.last_column);
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0); setStatementLocation($$.m_node, @1, @3); }
  | BREAK IDENT error                   { BreakNode* node = new BreakNode(GLOBAL_DATA, *$2);
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @2.last_column);
                                          $$ = createNodeDeclarationInfo<StatementNode*>(new BreakNode(GLOBAL_DATA, *$2), 0, 0, 0, 0); setStatementLocation($$.m_node, @1, @2); AUTO_SEMICOLON; }
;

ReturnStatement:
    RETURN SEMICOLON                          { ReturnNode* node = new ReturnNode(GLOBAL_DATA, 0); 
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @1.last_column); 
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0); setStatementLocation($$.m_node, @1, @2); }
  | RETURN error                        { ReturnNode* node = new ReturnNode(GLOBAL_DATA, 0); 
                                          setExceptionLocation(node, @1.first_column, @1.last_column, @1.last_column); 
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0); setStatementLocation($$.m_node, @1, @1); AUTO_SEMICOLON; }
  | RETURN Expr SEMICOLON                     { ReturnNode* node = new ReturnNode(GLOBAL_DATA, $2.m_node); 
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @2.last_column);
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, $2.m_features, $2.m_numConstants); setStatementLocation($$.m_node, @1, @3); }
  | RETURN Expr error                   { ReturnNode* node = new ReturnNode(GLOBAL_DATA, $2.m_node); 
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @2.last_column); 
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, $2.m_features, $2.m_numConstants); setStatementLocation($$.m_node, @1, @2); AUTO_SEMICOLON; }
;

WithStatement:
    WITH LPAREN Expr RPAREN Statement         { $$ = createNodeDeclarationInfo<StatementNode*>(new WithNode(GLOBAL_DATA, $3.m_node, $5.m_node, @3.last_column, @3.last_column - @3.first_column),
                                                                                         $5.m_varDeclarations, $5.m_funcDeclarations, $3.m_features | $5.m_features | WithFeature, $3.m_numConstants + $5.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @4); }
;

SwitchStatement:
    SWITCH LPAREN Expr RPAREN CaseBlock       { $$ = createNodeDeclarationInfo<StatementNode*>(new SwitchNode(GLOBAL_DATA, $3.m_node, $5.m_node), $5.m_varDeclarations, $5.m_funcDeclarations,
                                                                                         $3.m_features | $5.m_features, $3.m_numConstants + $5.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @4); }
;

CaseBlock:
    OPENBRACE CaseClausesOpt CLOSEBRACE              { $$ = createNodeDeclarationInfo<CaseBlockNode*>(new CaseBlockNode(GLOBAL_DATA, $2.m_node.head, 0, 0), $2.m_varDeclarations, $2.m_funcDeclarations, $2.m_features, $2.m_numConstants); }
  | OPENBRACE CaseClausesOpt DefaultClause CaseClausesOpt CLOSEBRACE
                                        { $$ = createNodeDeclarationInfo<CaseBlockNode*>(new CaseBlockNode(GLOBAL_DATA, $2.m_node.head, $3.m_node, $4.m_node.head),
                                                                                         mergeDeclarationLists(mergeDeclarationLists($2.m_varDeclarations, $3.m_varDeclarations), $4.m_varDeclarations),
                                                                                         mergeDeclarationLists(mergeDeclarationLists($2.m_funcDeclarations, $3.m_funcDeclarations), $4.m_funcDeclarations),
                                                                                         $2.m_features | $3.m_features | $4.m_features,
                                                                                         $2.m_numConstants + $3.m_numConstants + $4.m_numConstants); }
;

CaseClausesOpt:
  /* nothing */                         { $$.m_node.head = 0; $$.m_node.tail = 0; $$.m_varDeclarations = 0; $$.m_funcDeclarations = 0; $$.m_features = 0; $$.m_numConstants = 0; }
  | CaseClauses
;

CaseClauses:
    CaseClause                          { $$.m_node.head = new ClauseListNode(GLOBAL_DATA, $1.m_node);
                                          $$.m_node.tail = $$.m_node.head;
                                          $$.m_varDeclarations = $1.m_varDeclarations;
                                          $$.m_funcDeclarations = $1.m_funcDeclarations; 
                                          $$.m_features = $1.m_features;
                                          $$.m_numConstants = $1.m_numConstants; }
  | CaseClauses CaseClause              { $$.m_node.head = $1.m_node.head;
                                          $$.m_node.tail = new ClauseListNode(GLOBAL_DATA, $2.m_node, $1.m_node.tail);
                                          $$.m_varDeclarations = mergeDeclarationLists($1.m_varDeclarations, $2.m_varDeclarations);
                                          $$.m_funcDeclarations = mergeDeclarationLists($1.m_funcDeclarations, $2.m_funcDeclarations);
                                          $$.m_features = $1.m_features | $2.m_features;
                                          $$.m_numConstants = $1.m_numConstants + $2.m_numConstants;
                                        }
;

CaseClause:
    CASE Expr COLON                       { $$ = createNodeDeclarationInfo<CaseClauseNode*>(new CaseClauseNode(GLOBAL_DATA, $2.m_node), 0, 0, $2.m_features, $2.m_numConstants); }
  | CASE Expr COLON SourceElements        { $$ = createNodeDeclarationInfo<CaseClauseNode*>(new CaseClauseNode(GLOBAL_DATA, $2.m_node, $4.m_node), $4.m_varDeclarations, $4.m_funcDeclarations, $2.m_features | $4.m_features, $2.m_numConstants + $4.m_numConstants); }
;

DefaultClause:
    DEFAULT COLON                         { $$ = createNodeDeclarationInfo<CaseClauseNode*>(new CaseClauseNode(GLOBAL_DATA, 0), 0, 0, 0, 0); }
  | DEFAULT COLON SourceElements          { $$ = createNodeDeclarationInfo<CaseClauseNode*>(new CaseClauseNode(GLOBAL_DATA, 0, $3.m_node), $3.m_varDeclarations, $3.m_funcDeclarations, $3.m_features, $3.m_numConstants); }
;

LabelledStatement:
    IDENT COLON Statement                 { LabelNode* node = new LabelNode(GLOBAL_DATA, *$1, $3.m_node);
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @2.last_column);
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, $3.m_varDeclarations, $3.m_funcDeclarations, $3.m_features, $3.m_numConstants); }
;

ThrowStatement:
    THROW Expr SEMICOLON                      { ThrowNode* node = new ThrowNode(GLOBAL_DATA, $2.m_node);
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @2.last_column);
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, $2.m_features, $2.m_numConstants); setStatementLocation($$.m_node, @1, @2);
                                        }
  | THROW Expr error                    { ThrowNode* node = new ThrowNode(GLOBAL_DATA, $2.m_node);
                                          setExceptionLocation(node, @1.first_column, @2.last_column, @2.last_column);
                                          $$ = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, $2.m_features, $2.m_numConstants); setStatementLocation($$.m_node, @1, @2); AUTO_SEMICOLON; 
                                        }
;

TryStatement:
    TRY Block FINALLY Block             { $$ = createNodeDeclarationInfo<StatementNode*>(new TryNode(GLOBAL_DATA, $2.m_node, GLOBAL_DATA->propertyNames->nullIdentifier, false, 0, $4.m_node),
                                                                                         mergeDeclarationLists($2.m_varDeclarations, $4.m_varDeclarations),
                                                                                         mergeDeclarationLists($2.m_funcDeclarations, $4.m_funcDeclarations),
                                                                                         $2.m_features | $4.m_features,
                                                                                         $2.m_numConstants + $4.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @2); }
  | TRY Block CATCH LPAREN IDENT RPAREN Block { $$ = createNodeDeclarationInfo<StatementNode*>(new TryNode(GLOBAL_DATA, $2.m_node, *$5, ($7.m_features & EvalFeature) != 0, $7.m_node, 0),
                                                                                         mergeDeclarationLists($2.m_varDeclarations, $7.m_varDeclarations),
                                                                                         mergeDeclarationLists($2.m_funcDeclarations, $7.m_funcDeclarations),
                                                                                         $2.m_features | $7.m_features | CatchFeature,
                                                                                         $2.m_numConstants + $7.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @2); }
  | TRY Block CATCH LPAREN IDENT RPAREN Block FINALLY Block
                                        { $$ = createNodeDeclarationInfo<StatementNode*>(new TryNode(GLOBAL_DATA, $2.m_node, *$5, ($7.m_features & EvalFeature) != 0, $7.m_node, $9.m_node),
                                                                                         mergeDeclarationLists(mergeDeclarationLists($2.m_varDeclarations, $7.m_varDeclarations), $9.m_varDeclarations),
                                                                                         mergeDeclarationLists(mergeDeclarationLists($2.m_funcDeclarations, $7.m_funcDeclarations), $9.m_funcDeclarations),
                                                                                         $2.m_features | $7.m_features | $9.m_features | CatchFeature,
                                                                                         $2.m_numConstants + $7.m_numConstants + $9.m_numConstants);
                                          setStatementLocation($$.m_node, @1, @2); }
;

DebuggerStatement:
    DEBUGGER SEMICOLON                        { $$ = createNodeDeclarationInfo<StatementNode*>(new DebuggerStatementNode(GLOBAL_DATA), 0, 0, 0, 0);
                                          setStatementLocation($$.m_node, @1, @2); }
  | DEBUGGER error                      { $$ = createNodeDeclarationInfo<StatementNode*>(new DebuggerStatementNode(GLOBAL_DATA), 0, 0, 0, 0);
                                          setStatementLocation($$.m_node, @1, @1); AUTO_SEMICOLON; }
;

FunctionDeclaration:
    FUNCTION IDENT LPAREN RPAREN OPENBRACE FunctionBody CLOSEBRACE { $$ = createNodeDeclarationInfo<StatementNode*>(new FuncDeclNode(GLOBAL_DATA, *$2, $6), 0, new ParserArenaData<DeclarationStacks::FunctionStack>, ((*$2 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | ClosureFeature, 0); setStatementLocation($6, @5, @7); $$.m_funcDeclarations->data.push_back(static_cast<FuncDeclNode*>($$.m_node)->body()); }
  | FUNCTION IDENT LPAREN FormalParameterList RPAREN OPENBRACE FunctionBody CLOSEBRACE
      {
          $$ = createNodeDeclarationInfo<StatementNode*>(new FuncDeclNode(GLOBAL_DATA, *$2, $7, $4.m_node.head), 0, new ParserArenaData<DeclarationStacks::FunctionStack>, ((*$2 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | $4.m_features | ClosureFeature, 0);
          if ($4.m_features & ArgumentsFeature)
              $7->setUsesArguments();
          setStatementLocation($7, @6, @8);
          $$.m_funcDeclarations->data.push_back(static_cast<FuncDeclNode*>($$.m_node)->body());
      }
;

FunctionExpr:
    FUNCTION LPAREN RPAREN OPENBRACE FunctionBody CLOSEBRACE { $$ = createNodeInfo(new FuncExprNode(GLOBAL_DATA, GLOBAL_DATA->propertyNames->nullIdentifier, $5), ClosureFeature, 0); setStatementLocation($5, @4, @6); }
    | FUNCTION LPAREN FormalParameterList RPAREN OPENBRACE FunctionBody CLOSEBRACE
      {
          $$ = createNodeInfo(new FuncExprNode(GLOBAL_DATA, GLOBAL_DATA->propertyNames->nullIdentifier, $6, $3.m_node.head), $3.m_features | ClosureFeature, 0);
          if ($3.m_features & ArgumentsFeature)
              $6->setUsesArguments();
          setStatementLocation($6, @5, @7);
      }
  | FUNCTION IDENT LPAREN RPAREN OPENBRACE FunctionBody CLOSEBRACE { $$ = createNodeInfo(new FuncExprNode(GLOBAL_DATA, *$2, $6), ClosureFeature, 0); setStatementLocation($6, @5, @7); }
  | FUNCTION IDENT LPAREN FormalParameterList RPAREN OPENBRACE FunctionBody CLOSEBRACE
      {
          $$ = createNodeInfo(new FuncExprNode(GLOBAL_DATA, *$2, $7, $4.m_node.head), $4.m_features | ClosureFeature, 0); 
          if ($4.m_features & ArgumentsFeature)
              $7->setUsesArguments();
          setStatementLocation($7, @6, @8);
      }
;

FormalParameterList:
    IDENT                               { $$.m_node.head = new ParameterNode(GLOBAL_DATA, *$1);
                                          $$.m_features = (*$1 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0;
                                          $$.m_node.tail = $$.m_node.head; }
  | FormalParameterList COMMA IDENT       { $$.m_node.head = $1.m_node.head;
                                          $$.m_features = $1.m_features | ((*$3 == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0);
                                          $$.m_node.tail = new ParameterNode(GLOBAL_DATA, *$3, $1.m_node.tail);  }
;

FunctionBody:
    /* not in spec */                   { $$ = FunctionBodyNode::create(GLOBAL_DATA); }
  | SourceElements                      { $$ = FunctionBodyNode::create(GLOBAL_DATA, $1.m_node, $1.m_varDeclarations ? &$1.m_varDeclarations->data : 0, 
                                                       $1.m_funcDeclarations ? &$1.m_funcDeclarations->data : 0,
                                                       ($1.m_features & EvalFeature) != 0, ($1.m_features & ClosureFeature) != 0);
                                        }
;

Program:
    /* not in spec */                   { }
    | SourceElements                    { programBlock = $1.m_node; }
;

SourceElements:
    Statement                           { $$.m_node = new SourceElements(GLOBAL_DATA);
                                          $$.m_node->append($1.m_node);
                                          $$.m_varDeclarations = $1.m_varDeclarations;
                                          $$.m_funcDeclarations = $1.m_funcDeclarations;
                                          $$.m_features = $1.m_features;
                                          $$.m_numConstants = $1.m_numConstants;
                                        }
  | SourceElements Statement            { $$.m_node->append($2.m_node);
                                          $$.m_varDeclarations = mergeDeclarationLists($1.m_varDeclarations, $2.m_varDeclarations);
                                          $$.m_funcDeclarations = mergeDeclarationLists($1.m_funcDeclarations, $2.m_funcDeclarations);
                                          $$.m_features = $1.m_features | $2.m_features;
                                          $$.m_numConstants = $1.m_numConstants + $2.m_numConstants;
                                        }
;

%%

#undef GLOBAL_DATA

static ExpressionNode* makeAssignNode(CodeGenContext* globalData, ExpressionNode* loc, OpcodeID op, ExpressionNode* expr, bool locHasAssignments, bool exprHasAssignments, int start, int divot, int end)
{
    if (!loc->isLocation())
        return new AssignErrorNode(globalData, loc, op, expr, divot, divot - start, end - divot);

    if (loc->isResolveNode()) {
        ResolveNode* resolve = static_cast<ResolveNode*>(loc);
        if (op == op_eq) {
            AssignResolveNode* node = new AssignResolveNode(globalData, resolve->identifier(), expr, exprHasAssignments);
            setExceptionLocation(node, start, divot, end);
            return node;
        } else
            return new ReadModifyResolveNode(globalData, resolve->identifier(), op, expr, exprHasAssignments, divot, divot - start, end - divot);
    }
    if (loc->isBracketAccessorNode()) {
        BracketAccessorNode* bracket = static_cast<BracketAccessorNode*>(loc);
        if (op == op_eq)
            return new AssignBracketNode(globalData, bracket->base(), bracket->subscript(), expr, locHasAssignments, exprHasAssignments, bracket->divot(), bracket->divot() - start, end - bracket->divot());
        else {
            ReadModifyBracketNode* node = new ReadModifyBracketNode(globalData, bracket->base(), bracket->subscript(), op, expr, locHasAssignments, exprHasAssignments, divot, divot - start, end - divot);
            node->setSubexpressionInfo(bracket->divot(), bracket->endOffset());
            return node;
        }
    }
    ASSERT(loc->isDotAccessorNode());
    DotAccessorNode* dot = static_cast<DotAccessorNode*>(loc);
    if (op == op_eq)
        return new AssignDotNode(globalData, dot->base(), dot->identifier(), expr, exprHasAssignments, dot->divot(), dot->divot() - start, end - dot->divot());

    ReadModifyDotNode* node = new ReadModifyDotNode(globalData, dot->base(), dot->identifier(), op, expr, exprHasAssignments, divot, divot - start, end - divot);
    node->setSubexpressionInfo(dot->divot(), dot->endOffset());
    return node;
}

static ExpressionNode* makePrefixNode(CodeGenContext* globalData, ExpressionNode* expr, OpcodeID op, int start, int divot, int end)
{
    if (!expr->isLocation())
        return new PrefixErrorNode(globalData, expr, op, divot, divot - start, end - divot);
    
    if (expr->isResolveNode()) {
        ResolveNode* resolve = static_cast<ResolveNode*>(expr);
        return new PrefixResolveNode(globalData, resolve->identifier(), op, divot, divot - start, end - divot);
    }
    if (expr->isBracketAccessorNode()) {
        BracketAccessorNode* bracket = static_cast<BracketAccessorNode*>(expr);
        PrefixBracketNode* node = new PrefixBracketNode(globalData, bracket->base(), bracket->subscript(), op, divot, divot - start, end - divot);
        node->setSubexpressionInfo(bracket->divot(), bracket->startOffset());
        return node;
    }
    ASSERT(expr->isDotAccessorNode());
    DotAccessorNode* dot = static_cast<DotAccessorNode*>(expr);
    PrefixDotNode* node = new PrefixDotNode(globalData, dot->base(), dot->identifier(), op, divot, divot - start, end - divot);
    node->setSubexpressionInfo(dot->divot(), dot->startOffset());
    return node;
}

static ExpressionNode* makePostfixNode(CodeGenContext* globalData, ExpressionNode* expr, OpcodeID op, int start, int divot, int end)
{ 
    if (!expr->isLocation())
        return new PostfixErrorNode(globalData, expr, op, divot, divot - start, end - divot);
    
    if (expr->isResolveNode()) {
        ResolveNode* resolve = static_cast<ResolveNode*>(expr);
        return new PostfixResolveNode(globalData, resolve->identifier(), op, divot, divot - start, end - divot);
    }
    if (expr->isBracketAccessorNode()) {
        BracketAccessorNode* bracket = static_cast<BracketAccessorNode*>(expr);
        PostfixBracketNode* node = new PostfixBracketNode(globalData, bracket->base(), bracket->subscript(), op, divot, divot - start, end - divot);
        node->setSubexpressionInfo(bracket->divot(), bracket->endOffset());
        return node;
        
    }
    ASSERT(expr->isDotAccessorNode());
    DotAccessorNode* dot = static_cast<DotAccessorNode*>(expr);
    PostfixDotNode* node = new PostfixDotNode(globalData, dot->base(), dot->identifier(), op, divot, divot - start, end - divot);
    node->setSubexpressionInfo(dot->divot(), dot->endOffset());
    return node;
}

static ExpressionNodeInfo makeFunctionCallNode(CodeGenContext* globalData, ExpressionNodeInfo func, ArgumentsNodeInfo args, int start, int divot, int end)
{
    CodeFeatures features = func.m_features | args.m_features;
    int numConstants = func.m_numConstants + args.m_numConstants;
    if (!func.m_node->isLocation())
        return createNodeInfo<ExpressionNode*>(new FunctionCallValueNode(globalData, func.m_node, args.m_node, divot, divot - start, end - divot), features, numConstants);
    if (func.m_node->isResolveNode()) {
        ResolveNode* resolve = static_cast<ResolveNode*>(func.m_node);
        const Identifier& identifier = resolve->identifier();
        if (identifier == globalData->propertyNames->eval)
            return createNodeInfo<ExpressionNode*>(new EvalFunctionCallNode(globalData, args.m_node, divot, divot - start, end - divot), EvalFeature | features, numConstants);
        return createNodeInfo<ExpressionNode*>(new FunctionCallResolveNode(globalData, identifier, args.m_node, divot, divot - start, end - divot), features, numConstants);
    }
    if (func.m_node->isBracketAccessorNode()) {
        BracketAccessorNode* bracket = static_cast<BracketAccessorNode*>(func.m_node);
        FunctionCallBracketNode* node = new FunctionCallBracketNode(globalData, bracket->base(), bracket->subscript(), args.m_node, divot, divot - start, end - divot);
        node->setSubexpressionInfo(bracket->divot(), bracket->endOffset());
        return createNodeInfo<ExpressionNode*>(node, features, numConstants);
    }
    ASSERT(func.m_node->isDotAccessorNode());
    DotAccessorNode* dot = static_cast<DotAccessorNode*>(func.m_node);
    FunctionCallDotNode* node;
    if (dot->identifier() == globalData->propertyNames->call)
        node = new CallFunctionCallDotNode(globalData, dot->base(), dot->identifier(), args.m_node, divot, divot - start, end - divot);
    else if (dot->identifier() == globalData->propertyNames->apply)
        node = new ApplyFunctionCallDotNode(globalData, dot->base(), dot->identifier(), args.m_node, divot, divot - start, end - divot);
    else
        node = new FunctionCallDotNode(globalData, dot->base(), dot->identifier(), args.m_node, divot, divot - start, end - divot);
    node->setSubexpressionInfo(dot->divot(), dot->endOffset());
    return createNodeInfo<ExpressionNode*>(node, features, numConstants);
}

static ExpressionNode* makeTypeOfNode(CodeGenContext* globalData, ExpressionNode* expr)
{
    if (expr->isResolveNode()) {
        ResolveNode* resolve = static_cast<ResolveNode*>(expr);
        return new TypeOfResolveNode(globalData, resolve->identifier());
    }
    return new TypeOfValueNode(globalData, expr);
}

static ExpressionNode* makeDeleteNode(CodeGenContext* globalData, ExpressionNode* expr, int start, int divot, int end)
{
    if (!expr->isLocation())
        return new DeleteValueNode(globalData, expr);
    if (expr->isResolveNode()) {
        ResolveNode* resolve = static_cast<ResolveNode*>(expr);
        return new DeleteResolveNode(globalData, resolve->identifier(), divot, divot - start, end - divot);
    }
    if (expr->isBracketAccessorNode()) {
        BracketAccessorNode* bracket = static_cast<BracketAccessorNode*>(expr);
        return new DeleteBracketNode(globalData, bracket->base(), bracket->subscript(), divot, divot - start, end - divot);
    }
    ASSERT(expr->isDotAccessorNode());
    DotAccessorNode* dot = static_cast<DotAccessorNode*>(expr);
    return new DeleteDotNode(globalData, dot->base(), dot->identifier(), divot, divot - start, end - divot);
}

static PropertyNode* makeGetterOrSetterPropertyNode(CodeGenContext* globalData, const Identifier& getOrSet, const Identifier& name, ParameterNode* params, FunctionBodyNode* body)
{
    PropertyNode::Type type;
    if (getOrSet == "get")
        type = PropertyNode::Getter;
    else if (getOrSet == "set")
        type = PropertyNode::Setter;
    else
        return 0;
    return new PropertyNode(globalData, name, new FuncExprNode(globalData, globalData->propertyNames->nullIdentifier, body, params), type);
}

static ExpressionNode* makeNegateNode(CodeGenContext* globalData, ExpressionNode* n)
{
    if (n->isNumber()) {
        NumberNode* number = static_cast<NumberNode*>(n);

        if (number->value() > 0.0) {
            number->setValue(-number->value());
            return number;
        }
    }

    return new NegateNode(globalData, n);
}

static NumberNode* makeNumberNode(CodeGenContext* globalData, double d)
{
    return new NumberNode(globalData, d);
}

static ExpressionNode* makeBitwiseNotNode(CodeGenContext* globalData, ExpressionNode* expr)
{
    if (expr->isNumber())
        return makeNumberNode(globalData, ~toInt32(static_cast<NumberNode*>(expr)->value()));
    return new BitwiseNotNode(globalData, expr);
}

static ExpressionNode* makeMultNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
{
    expr1 = expr1->stripUnaryPlus();
    expr2 = expr2->stripUnaryPlus();

    if (expr1->isNumber() && expr2->isNumber())
        return makeNumberNode(globalData, static_cast<NumberNode*>(expr1)->value() * static_cast<NumberNode*>(expr2)->value());

    if (expr1->isNumber() && static_cast<NumberNode*>(expr1)->value() == 1)
        return new UnaryPlusNode(globalData, expr2);

    if (expr2->isNumber() && static_cast<NumberNode*>(expr2)->value() == 1)
        return new UnaryPlusNode(globalData, expr1);

    return new MultNode(globalData, expr1, expr2, rightHasAssignments);
}

static ExpressionNode* makeDivNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
{
    expr1 = expr1->stripUnaryPlus();
    expr2 = expr2->stripUnaryPlus();

    if (expr1->isNumber() && expr2->isNumber())
        return makeNumberNode(globalData, static_cast<NumberNode*>(expr1)->value() / static_cast<NumberNode*>(expr2)->value());
    return new DivNode(globalData, expr1, expr2, rightHasAssignments);
}

static ExpressionNode* makeAddNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
{
    if (expr1->isNumber() && expr2->isNumber())
        return makeNumberNode(globalData, static_cast<NumberNode*>(expr1)->value() + static_cast<NumberNode*>(expr2)->value());
    return new AddNode(globalData, expr1, expr2, rightHasAssignments);
}

static ExpressionNode* makeSubNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
{
    expr1 = expr1->stripUnaryPlus();
    expr2 = expr2->stripUnaryPlus();

    if (expr1->isNumber() && expr2->isNumber())
        return makeNumberNode(globalData, static_cast<NumberNode*>(expr1)->value() - static_cast<NumberNode*>(expr2)->value());
    return new SubNode(globalData, expr1, expr2, rightHasAssignments);
}

static ExpressionNode* makeLeftShiftNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
{
    if (expr1->isNumber() && expr2->isNumber())
        return makeNumberNode(globalData, toInt32(static_cast<NumberNode*>(expr1)->value()) << (toUInt32(static_cast<NumberNode*>(expr2)->value()) & 0x1f));
    return new LeftShiftNode(globalData, expr1, expr2, rightHasAssignments);
}

static ExpressionNode* makeRightShiftNode(CodeGenContext* globalData, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments)
{
    if (expr1->isNumber() && expr2->isNumber())
        return makeNumberNode(globalData, toInt32(static_cast<NumberNode*>(expr1)->value()) >> (toUInt32(static_cast<NumberNode*>(expr2)->value()) & 0x1f));
    return new RightShiftNode(globalData, expr1, expr2, rightHasAssignments);
}

// Called by yyparse on error.
int yyerror(const char *s)
{
    std::printf("Error: %s\n", s);
    return 1;
}

static ExpressionNode* combineCommaNodes(CodeGenContext* globalData, ExpressionNode* list, ExpressionNode* init)
{
    if (!list)
        return init;
    if (list->isCommaNode()) {
        static_cast<CommaNode*>(list)->append(init);
        return list;
    }
    return new CommaNode(globalData, list, init);
}

// We turn variable declarations into either assignments or empty
// statements (which later get stripped out), because the actual
// declaration work is hoisted up to the start of the function body
static StatementNode* makeVarStatementNode(CodeGenContext* globalData, ExpressionNode* expr)
{
    if (!expr)
        return new EmptyStatementNode(globalData);
    return new VarStatementNode(globalData, expr);
}

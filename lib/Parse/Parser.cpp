
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "..\\lib\\Parse\\ecmascript.y"


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



/* Line 189 of yacc.c  */
#line 213 "..\\lib\\Parse\\Parser.cpp"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NULLTOKEN = 258,
     TRUETOKEN = 259,
     FALSETOKEN = 260,
     BREAK = 261,
     CASE = 262,
     DEFAULT = 263,
     FOR = 264,
     NEW = 265,
     VAR = 266,
     CONSTTOKEN = 267,
     CONTINUE = 268,
     FUNCTION = 269,
     RETURN = 270,
     VOIDTOKEN = 271,
     DELETETOKEN = 272,
     IF = 273,
     THISTOKEN = 274,
     DO = 275,
     WHILE = 276,
     INTOKEN = 277,
     INSTANCEOF = 278,
     TYPEOF = 279,
     SWITCH = 280,
     WITH = 281,
     RESERVED = 282,
     THROW = 283,
     TRY = 284,
     CATCH = 285,
     FINALLY = 286,
     DEBUGGER = 287,
     IF_WITHOUT_ELSE = 288,
     ELSE = 289,
     EQEQ = 290,
     NE = 291,
     STREQ = 292,
     STRNEQ = 293,
     LE = 294,
     GE = 295,
     OR = 296,
     AND = 297,
     PLUSPLUS = 298,
     MINUSMINUS = 299,
     LSHIFT = 300,
     RSHIFT = 301,
     URSHIFT = 302,
     PLUSEQUAL = 303,
     MINUSEQUAL = 304,
     MULTEQUAL = 305,
     DIVEQUAL = 306,
     LSHIFTEQUAL = 307,
     RSHIFTEQUAL = 308,
     URSHIFTEQUAL = 309,
     ANDEQUAL = 310,
     MODEQUAL = 311,
     XOREQUAL = 312,
     OREQUAL = 313,
     OPENBRACE = 314,
     CLOSEBRACE = 315,
     LT = 316,
     GT = 317,
     EQUAL = 318,
     NOT = 319,
     BITOR = 320,
     BITAND = 321,
     BITXOR = 322,
     BITINV = 323,
     LPAREN = 324,
     RPAREN = 325,
     LBRACKET = 326,
     RBRACKET = 327,
     QUESTIONMARK = 328,
     DOT = 329,
     COLON = 330,
     SEMICOLON = 331,
     COMMA = 332,
     PLUS = 333,
     MINUS = 334,
     MUL = 335,
     DIV = 336,
     MOD = 337,
     NUMBER = 338,
     IDENT = 339,
     STRING = 340,
     REGEXP = 341,
     AUTOPLUSPLUS = 342,
     AUTOMINUSMINUS = 343
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 140 "..\\lib\\Parse\\ecmascript.y"

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



/* Line 214 of yacc.c  */
#line 371 "..\\lib\\Parse\\Parser.cpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */

/* Line 264 of yacc.c  */
#line 172 "..\\lib\\Parse\\ecmascript.y"


template <typename T> inline void setStatementLocation(StatementNode* statement, const T& start, const T& end)
{
    statement->setLoc(start.first_line, end.last_line);
}

static inline void setExceptionLocation(ThrowableExpressionData* node, unsigned start, unsigned divot, unsigned end)
{
    node->setExceptionSourceCode(divot, divot - start, end - divot);
}



/* Line 264 of yacc.c  */
#line 411 "..\\lib\\Parse\\Parser.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  205
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1204

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  98
/* YYNRULES -- Number of rules.  */
#define YYNRULES  299
/* YYNRULES -- Number of states.  */
#define YYNSTATES  542

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   343

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    19,
      23,    27,    35,    44,    46,    50,    52,    55,    59,    64,
      66,    68,    70,    72,    76,    80,    84,    90,    93,    98,
      99,   101,   103,   106,   108,   110,   115,   119,   123,   125,
     130,   134,   138,   140,   143,   145,   148,   151,   154,   159,
     163,   166,   169,   174,   178,   181,   185,   187,   191,   193,
     195,   197,   199,   201,   204,   207,   209,   212,   215,   218,
     221,   224,   227,   230,   233,   236,   239,   242,   245,   248,
     250,   252,   254,   256,   258,   262,   266,   270,   272,   276,
     280,   284,   286,   290,   294,   296,   300,   304,   306,   310,
     314,   318,   320,   324,   328,   332,   334,   338,   342,   346,
     350,   354,   358,   360,   364,   368,   372,   376,   380,   382,
     386,   390,   394,   398,   402,   406,   408,   412,   416,   420,
     424,   426,   430,   434,   438,   442,   444,   448,   452,   456,
     460,   462,   466,   468,   472,   474,   478,   480,   484,   486,
     490,   492,   496,   498,   502,   504,   508,   510,   514,   516,
     520,   522,   526,   528,   532,   534,   538,   540,   544,   546,
     550,   552,   558,   560,   566,   568,   574,   576,   580,   582,
     586,   588,   592,   594,   596,   598,   600,   602,   604,   606,
     608,   610,   612,   614,   616,   618,   622,   624,   628,   630,
     634,   636,   638,   640,   642,   644,   646,   648,   650,   652,
     654,   656,   658,   660,   662,   664,   666,   668,   671,   675,
     679,   683,   685,   688,   692,   697,   699,   702,   706,   711,
     715,   719,   721,   725,   727,   730,   733,   736,   738,   741,
     744,   750,   758,   766,   774,   780,   790,   801,   809,   818,
     828,   829,   831,   832,   834,   837,   840,   844,   848,   851,
     854,   858,   862,   865,   868,   872,   876,   882,   888,   892,
     898,   899,   901,   903,   906,   910,   915,   918,   922,   926,
     930,   934,   939,   947,   957,   960,   963,   971,   980,   987,
     995,  1003,  1012,  1014,  1018,  1019,  1021,  1022,  1024,  1026
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     185,     0,    -1,     3,    -1,     4,    -1,     5,    -1,    83,
      -1,    85,    -1,    86,    -1,    84,    75,   144,    -1,    85,
      75,   144,    -1,    83,    75,   144,    -1,    84,    84,    69,
      70,    59,   184,    60,    -1,    84,    84,    69,   183,    70,
      59,   184,    60,    -1,    91,    -1,    92,    77,    91,    -1,
      94,    -1,    59,    60,    -1,    59,    92,    60,    -1,    59,
      92,    77,    60,    -1,    19,    -1,    90,    -1,    95,    -1,
      84,    -1,    69,   148,    70,    -1,    71,    97,    72,    -1,
      71,    96,    72,    -1,    71,    96,    77,    97,    72,    -1,
      97,   144,    -1,    96,    77,    97,   144,    -1,    -1,    98,
      -1,    77,    -1,    98,    77,    -1,    93,    -1,   182,    -1,
      99,    71,   148,    72,    -1,    99,    74,    84,    -1,    10,
      99,   105,    -1,    94,    -1,   100,    71,   148,    72,    -1,
     100,    74,    84,    -1,    10,    99,   105,    -1,    99,    -1,
      10,   101,    -1,   100,    -1,    10,   101,    -1,    99,   105,
      -1,   103,   105,    -1,   103,    71,   148,    72,    -1,   103,
      74,    84,    -1,   100,   105,    -1,   104,   105,    -1,   104,
      71,   148,    72,    -1,   104,    74,    84,    -1,    69,    70,
      -1,    69,   106,    70,    -1,   144,    -1,   106,    77,   144,
      -1,   101,    -1,   103,    -1,   102,    -1,   104,    -1,   107,
      -1,   107,    43,    -1,   107,    44,    -1,   108,    -1,   108,
      43,    -1,   108,    44,    -1,    17,   112,    -1,    16,   112,
      -1,    24,   112,    -1,    43,   112,    -1,    87,   112,    -1,
      44,   112,    -1,    88,   112,    -1,    78,   112,    -1,    79,
     112,    -1,    68,   112,    -1,    64,   112,    -1,   109,    -1,
     111,    -1,   110,    -1,   111,    -1,   112,    -1,   114,    80,
     112,    -1,   114,    81,   112,    -1,   114,    82,   112,    -1,
     113,    -1,   115,    80,   112,    -1,   115,    81,   112,    -1,
     115,    82,   112,    -1,   114,    -1,   116,    78,   114,    -1,
     116,    79,   114,    -1,   115,    -1,   117,    78,   114,    -1,
     117,    79,   114,    -1,   116,    -1,   118,    45,   116,    -1,
     118,    46,   116,    -1,   118,    47,   116,    -1,   117,    -1,
     119,    45,   116,    -1,   119,    46,   116,    -1,   119,    47,
     116,    -1,   118,    -1,   120,    61,   118,    -1,   120,    62,
     118,    -1,   120,    39,   118,    -1,   120,    40,   118,    -1,
     120,    23,   118,    -1,   120,    22,   118,    -1,   118,    -1,
     121,    61,   118,    -1,   121,    62,   118,    -1,   121,    39,
     118,    -1,   121,    40,   118,    -1,   121,    23,   118,    -1,
     119,    -1,   122,    61,   118,    -1,   122,    62,   118,    -1,
     122,    39,   118,    -1,   122,    40,   118,    -1,   122,    23,
     118,    -1,   122,    22,   118,    -1,   120,    -1,   123,    35,
     120,    -1,   123,    36,   120,    -1,   123,    37,   120,    -1,
     123,    38,   120,    -1,   121,    -1,   124,    35,   121,    -1,
     124,    36,   121,    -1,   124,    37,   121,    -1,   124,    38,
     121,    -1,   122,    -1,   125,    35,   120,    -1,   125,    36,
     120,    -1,   125,    37,   120,    -1,   125,    38,   120,    -1,
     123,    -1,   126,    66,   123,    -1,   124,    -1,   127,    66,
     124,    -1,   125,    -1,   128,    66,   123,    -1,   126,    -1,
     129,    67,   126,    -1,   127,    -1,   130,    67,   127,    -1,
     128,    -1,   131,    67,   126,    -1,   129,    -1,   132,    65,
     129,    -1,   130,    -1,   133,    65,   130,    -1,   131,    -1,
     134,    65,   129,    -1,   132,    -1,   135,    42,   132,    -1,
     133,    -1,   136,    42,   133,    -1,   134,    -1,   137,    42,
     132,    -1,   135,    -1,   138,    41,   135,    -1,   136,    -1,
     139,    41,   136,    -1,   137,    -1,   140,    41,   135,    -1,
     138,    -1,   138,    73,   144,    75,   144,    -1,   139,    -1,
     139,    73,   145,    75,   145,    -1,   140,    -1,   140,    73,
     144,    75,   144,    -1,   141,    -1,   107,   147,   144,    -1,
     142,    -1,   107,   147,   145,    -1,   143,    -1,   108,   147,
     144,    -1,    63,    -1,    48,    -1,    49,    -1,    50,    -1,
      51,    -1,    52,    -1,    53,    -1,    54,    -1,    55,    -1,
      57,    -1,    58,    -1,    56,    -1,   144,    -1,   148,    77,
     144,    -1,   145,    -1,   149,    77,   145,    -1,   146,    -1,
     150,    77,   144,    -1,   152,    -1,   153,    -1,   156,    -1,
     181,    -1,   161,    -1,   162,    -1,   163,    -1,   164,    -1,
     167,    -1,   168,    -1,   169,    -1,   170,    -1,   171,    -1,
     177,    -1,   178,    -1,   179,    -1,   180,    -1,    59,    60,
      -1,    59,   186,    60,    -1,    11,   154,    76,    -1,    11,
     154,     1,    -1,    84,    -1,    84,   159,    -1,   154,    77,
      84,    -1,   154,    77,    84,   159,    -1,    84,    -1,    84,
     160,    -1,   155,    77,    84,    -1,   155,    77,    84,   160,
      -1,    12,   157,    76,    -1,    12,   157,     1,    -1,   158,
      -1,   157,    77,   158,    -1,    84,    -1,    84,   159,    -1,
      63,   144,    -1,    63,   145,    -1,    76,    -1,   150,    76,
      -1,   150,     1,    -1,    18,    69,   148,    70,   151,    -1,
      18,    69,   148,    70,   151,    34,   151,    -1,    20,   151,
      21,    69,   148,    70,    76,    -1,    20,   151,    21,    69,
     148,    70,     1,    -1,    21,    69,   148,    70,   151,    -1,
       9,    69,   166,    76,   165,    76,   165,    70,   151,    -1,
       9,    69,    11,   155,    76,   165,    76,   165,    70,   151,
      -1,     9,    69,   107,    22,   148,    70,   151,    -1,     9,
      69,    11,    84,    22,   148,    70,   151,    -1,     9,    69,
      11,    84,   160,    22,   148,    70,   151,    -1,    -1,   148,
      -1,    -1,   149,    -1,    13,    76,    -1,    13,     1,    -1,
      13,    84,    76,    -1,    13,    84,     1,    -1,     6,    76,
      -1,     6,     1,    -1,     6,    84,    76,    -1,     6,    84,
       1,    -1,    15,    76,    -1,    15,     1,    -1,    15,   148,
      76,    -1,    15,   148,     1,    -1,    26,    69,   148,    70,
     151,    -1,    25,    69,   148,    70,   172,    -1,    59,   173,
      60,    -1,    59,   173,   176,   173,    60,    -1,    -1,   174,
      -1,   175,    -1,   174,   175,    -1,     7,   148,    75,    -1,
       7,   148,    75,   186,    -1,     8,    75,    -1,     8,    75,
     186,    -1,    84,    75,   151,    -1,    28,   148,    76,    -1,
      28,   148,     1,    -1,    29,   152,    31,   152,    -1,    29,
     152,    30,    69,    84,    70,   152,    -1,    29,   152,    30,
      69,    84,    70,   152,    31,   152,    -1,    32,    76,    -1,
      32,     1,    -1,    14,    84,    69,    70,    59,   184,    60,
      -1,    14,    84,    69,   183,    70,    59,   184,    60,    -1,
      14,    69,    70,    59,   184,    60,    -1,    14,    69,   183,
      70,    59,   184,    60,    -1,    14,    84,    69,    70,    59,
     184,    60,    -1,    14,    84,    69,   183,    70,    59,   184,
      60,    -1,    84,    -1,   183,    77,    84,    -1,    -1,   186,
      -1,    -1,   186,    -1,   151,    -1,   186,   151,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   299,   299,   300,   301,   302,   303,   304,   316,   317,
     318,   319,   320,   332,   336,   343,   344,   345,   347,   351,
     352,   353,   354,   355,   359,   360,   361,   365,   369,   377,
     378,   382,   383,   387,   388,   389,   393,   397,   404,   405,
     409,   413,   420,   421,   428,   429,   436,   437,   438,   442,
     448,   449,   450,   454,   461,   462,   466,   470,   477,   478,
     482,   483,   487,   488,   489,   493,   494,   495,   499,   500,
     501,   502,   503,   504,   505,   506,   507,   508,   509,   512,
     513,   517,   518,   522,   523,   524,   525,   529,   530,   532,
     534,   539,   540,   541,   545,   546,   548,   553,   554,   555,
     556,   560,   561,   562,   563,   567,   568,   569,   570,   571,
     572,   575,   581,   582,   583,   584,   585,   586,   593,   594,
     595,   596,   597,   598,   602,   609,   610,   611,   612,   613,
     617,   618,   620,   622,   624,   629,   630,   632,   633,   635,
     640,   641,   645,   646,   651,   652,   656,   657,   661,   662,
     667,   668,   673,   674,   678,   679,   684,   685,   690,   691,
     695,   696,   701,   702,   707,   708,   712,   713,   718,   719,
     723,   724,   729,   730,   735,   736,   741,   742,   749,   750,
     757,   758,   765,   766,   767,   768,   769,   770,   771,   772,
     773,   774,   775,   776,   780,   781,   785,   786,   790,   791,
     795,   796,   797,   798,   799,   800,   801,   802,   803,   804,
     805,   806,   807,   808,   809,   810,   811,   815,   817,   822,
     824,   830,   838,   847,   857,   870,   877,   886,   894,   907,
     909,   915,   923,   935,   936,   940,   944,   948,   952,   954,
     959,   962,   972,   974,   976,   978,   984,   991,  1000,  1006,
    1017,  1018,  1022,  1023,  1027,  1031,  1035,  1039,  1046,  1049,
    1052,  1055,  1061,  1064,  1067,  1070,  1076,  1082,  1088,  1089,
    1098,  1099,  1103,  1109,  1119,  1120,  1124,  1125,  1129,  1135,
    1139,  1146,  1152,  1158,  1168,  1170,  1175,  1176,  1187,  1188,
    1195,  1196,  1206,  1209,  1215,  1216,  1223,  1224,  1228,  1235
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NULLTOKEN", "TRUETOKEN", "FALSETOKEN",
  "BREAK", "CASE", "DEFAULT", "FOR", "NEW", "VAR", "CONSTTOKEN",
  "CONTINUE", "FUNCTION", "RETURN", "VOIDTOKEN", "DELETETOKEN", "IF",
  "THISTOKEN", "DO", "WHILE", "INTOKEN", "INSTANCEOF", "TYPEOF", "SWITCH",
  "WITH", "RESERVED", "THROW", "TRY", "CATCH", "FINALLY", "DEBUGGER",
  "IF_WITHOUT_ELSE", "ELSE", "EQEQ", "NE", "STREQ", "STRNEQ", "LE", "GE",
  "OR", "AND", "PLUSPLUS", "MINUSMINUS", "LSHIFT", "RSHIFT", "URSHIFT",
  "PLUSEQUAL", "MINUSEQUAL", "MULTEQUAL", "DIVEQUAL", "LSHIFTEQUAL",
  "RSHIFTEQUAL", "URSHIFTEQUAL", "ANDEQUAL", "MODEQUAL", "XOREQUAL",
  "OREQUAL", "OPENBRACE", "CLOSEBRACE", "LT", "GT", "EQUAL", "NOT",
  "BITOR", "BITAND", "BITXOR", "BITINV", "LPAREN", "RPAREN", "LBRACKET",
  "RBRACKET", "QUESTIONMARK", "DOT", "COLON", "SEMICOLON", "COMMA", "PLUS",
  "MINUS", "MUL", "DIV", "MOD", "NUMBER", "IDENT", "STRING", "REGEXP",
  "AUTOPLUSPLUS", "AUTOMINUSMINUS", "$accept", "Literal", "Property",
  "PropertyList", "PrimaryExpr", "PrimaryExprNoBrace", "ArrayLiteral",
  "ElementList", "ElisionOpt", "Elision", "MemberExpr", "MemberExprNoBF",
  "NewExpr", "NewExprNoBF", "CallExpr", "CallExprNoBF", "Arguments",
  "ArgumentList", "LeftHandSideExpr", "LeftHandSideExprNoBF",
  "PostfixExpr", "PostfixExprNoBF", "UnaryExprCommon", "UnaryExpr",
  "UnaryExprNoBF", "MultiplicativeExpr", "MultiplicativeExprNoBF",
  "AdditiveExpr", "AdditiveExprNoBF", "ShiftExpr", "ShiftExprNoBF",
  "RelationalExpr", "RelationalExprNoIn", "RelationalExprNoBF",
  "EqualityExpr", "EqualityExprNoIn", "EqualityExprNoBF", "BitwiseANDExpr",
  "BitwiseANDExprNoIn", "BitwiseANDExprNoBF", "BitwiseXORExpr",
  "BitwiseXORExprNoIn", "BitwiseXORExprNoBF", "BitwiseORExpr",
  "BitwiseORExprNoIn", "BitwiseORExprNoBF", "LogicalANDExpr",
  "LogicalANDExprNoIn", "LogicalANDExprNoBF", "LogicalORExpr",
  "LogicalORExprNoIn", "LogicalORExprNoBF", "ConditionalExpr",
  "ConditionalExprNoIn", "ConditionalExprNoBF", "AssignmentExpr",
  "AssignmentExprNoIn", "AssignmentExprNoBF", "AssignmentOperator", "Expr",
  "ExprNoIn", "ExprNoBF", "Statement", "Block", "VariableStatement",
  "VariableDeclarationList", "VariableDeclarationListNoIn",
  "ConstStatement", "ConstDeclarationList", "ConstDeclaration",
  "Initializer", "InitializerNoIn", "EmptyStatement", "ExprStatement",
  "IfStatement", "IterationStatement", "ExprOpt", "ExprNoInOpt",
  "ContinueStatement", "BreakStatement", "ReturnStatement",
  "WithStatement", "SwitchStatement", "CaseBlock", "CaseClausesOpt",
  "CaseClauses", "CaseClause", "DefaultClause", "LabelledStatement",
  "ThrowStatement", "TryStatement", "DebuggerStatement",
  "FunctionDeclaration", "FunctionExpr", "FormalParameterList",
  "FunctionBody", "Program", "SourceElements", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    89,    90,    90,    90,    90,    90,    90,    91,    91,
      91,    91,    91,    92,    92,    93,    93,    93,    93,    94,
      94,    94,    94,    94,    95,    95,    95,    96,    96,    97,
      97,    98,    98,    99,    99,    99,    99,    99,   100,   100,
     100,   100,   101,   101,   102,   102,   103,   103,   103,   103,
     104,   104,   104,   104,   105,   105,   106,   106,   107,   107,
     108,   108,   109,   109,   109,   110,   110,   110,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   112,
     112,   113,   113,   114,   114,   114,   114,   115,   115,   115,
     115,   116,   116,   116,   117,   117,   117,   118,   118,   118,
     118,   119,   119,   119,   119,   120,   120,   120,   120,   120,
     120,   120,   121,   121,   121,   121,   121,   121,   122,   122,
     122,   122,   122,   122,   122,   123,   123,   123,   123,   123,
     124,   124,   124,   124,   124,   125,   125,   125,   125,   125,
     126,   126,   127,   127,   128,   128,   129,   129,   130,   130,
     131,   131,   132,   132,   133,   133,   134,   134,   135,   135,
     136,   136,   137,   137,   138,   138,   139,   139,   140,   140,
     141,   141,   142,   142,   143,   143,   144,   144,   145,   145,
     146,   146,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   148,   148,   149,   149,   150,   150,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   152,   152,   153,
     153,   154,   154,   154,   154,   155,   155,   155,   155,   156,
     156,   157,   157,   158,   158,   159,   160,   161,   162,   162,
     163,   163,   164,   164,   164,   164,   164,   164,   164,   164,
     165,   165,   166,   166,   167,   167,   167,   167,   168,   168,
     168,   168,   169,   169,   169,   169,   170,   171,   172,   172,
     173,   173,   174,   174,   175,   175,   176,   176,   177,   178,
     178,   179,   179,   179,   180,   180,   181,   181,   182,   182,
     182,   182,   183,   183,   184,   184,   185,   185,   186,   186
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     3,     3,
       3,     7,     8,     1,     3,     1,     2,     3,     4,     1,
       1,     1,     1,     3,     3,     3,     5,     2,     4,     0,
       1,     1,     2,     1,     1,     4,     3,     3,     1,     4,
       3,     3,     1,     2,     1,     2,     2,     2,     4,     3,
       2,     2,     4,     3,     2,     3,     1,     3,     1,     1,
       1,     1,     1,     2,     2,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       1,     1,     1,     1,     3,     3,     3,     1,     3,     3,
       3,     1,     3,     3,     1,     3,     3,     1,     3,     3,
       3,     1,     3,     3,     3,     1,     3,     3,     3,     3,
       3,     3,     1,     3,     3,     3,     3,     3,     1,     3,
       3,     3,     3,     3,     3,     1,     3,     3,     3,     3,
       1,     3,     3,     3,     3,     1,     3,     3,     3,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     5,     1,     5,     1,     5,     1,     3,     1,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     3,
       3,     1,     2,     3,     4,     1,     2,     3,     4,     3,
       3,     1,     3,     1,     2,     2,     2,     1,     2,     2,
       5,     7,     7,     7,     5,     9,    10,     7,     8,     9,
       0,     1,     0,     1,     2,     2,     3,     3,     2,     2,
       3,     3,     2,     2,     3,     3,     5,     5,     3,     5,
       0,     1,     1,     2,     3,     4,     2,     3,     3,     3,
       3,     4,     7,     9,     2,     2,     7,     8,     6,     7,
       7,     8,     1,     3,     0,     1,     0,     1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     296,     2,     3,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    19,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      29,   237,     0,     0,     5,    22,     6,     7,     0,     0,
      20,    38,    21,    44,    60,    61,    65,    81,    82,    87,
      94,   101,   118,   135,   144,   150,   156,   162,   168,   174,
     180,   198,     0,   298,   200,   201,   202,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     203,     0,   297,   259,   258,     0,   252,     0,     0,     0,
      22,    33,    15,    42,    45,    34,   221,     0,   233,     0,
     231,   255,   254,     0,     0,   263,   262,    42,    58,    59,
      62,    79,    80,    83,    91,    97,   105,   125,   140,   146,
     152,   158,   164,   170,   176,   194,     0,    62,    69,    68,
       0,     0,     0,    70,     0,     0,     0,     0,   285,   284,
      71,    73,   217,     0,    78,    77,     0,    31,     0,     0,
      30,    75,    76,     0,    72,    74,     0,     0,     0,    50,
       0,     0,    51,    66,    67,   183,   184,   185,   186,   187,
     188,   189,   190,   193,   191,   192,   182,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   239,   238,     0,     1,   299,   261,   260,     0,
      62,   112,   130,   142,   148,   154,   160,   166,   172,   178,
     196,   253,     0,    42,    43,     0,     0,    16,     0,     0,
       0,    13,     0,     0,     0,    41,     0,   222,   220,   219,
       0,   234,   230,   229,     0,   257,   256,     0,    46,     0,
       0,    47,    63,    64,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   265,
     264,     0,     0,     0,     0,     0,     0,   280,   279,     0,
       0,   218,    23,    25,    29,    24,    27,    32,   278,    54,
       0,    56,     0,    40,     0,    53,   181,    88,    89,    90,
      95,    96,   102,   103,   104,   124,   123,   121,   122,   119,
     120,   136,   137,   138,   139,   145,   151,   157,   163,   169,
       0,   199,   225,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   250,    37,     0,   292,     0,     0,     0,     0,
       0,     0,    17,     0,     0,    36,   235,   223,   232,     0,
       0,     0,    49,   177,    84,    85,    86,    92,    93,    98,
      99,   100,   111,   110,   108,   109,   106,   107,   126,   127,
     128,   129,   141,   147,   153,   159,   165,     0,   195,     0,
       0,     0,     0,     0,     0,   281,     0,    55,     0,    39,
      52,     0,     0,     0,   226,   250,     0,     0,    62,   179,
     117,   115,   116,   113,   114,   131,   132,   133,   134,   143,
     149,   155,   161,   167,     0,   197,   251,     0,   294,     0,
       0,     0,     0,    10,     8,     0,     9,    18,    14,    35,
     224,   294,     0,    48,     0,   240,     0,   244,   270,   267,
     266,     0,    26,    28,    57,   175,     0,   236,     0,     0,
     227,     0,     0,   250,     0,   295,   294,   293,   294,     0,
       0,     0,     0,   294,   171,     0,     0,     0,     0,   271,
     272,     0,     0,     0,   250,   228,   247,   173,     0,   288,
       0,     0,   294,   294,     0,   286,     0,   241,   243,   242,
       0,     0,   268,   270,   273,   282,   248,     0,     0,     0,
     289,   290,     0,     0,   294,   287,   274,   276,     0,     0,
     249,     0,   245,   291,    11,     0,   275,   277,   269,   283,
     246,    12
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    40,   231,   232,    91,    92,    42,   148,   149,   150,
     107,    43,   108,    44,   109,    45,   159,   300,   127,    46,
     111,    47,   112,   113,    49,   114,    50,   115,    51,   116,
      52,   117,   212,    53,   118,   213,    54,   119,   214,    55,
     120,   215,    56,   121,   216,    57,   122,   217,    58,   123,
     218,    59,   124,   219,    60,   125,   220,    61,   335,   436,
     221,    62,    63,    64,    65,    97,   333,    66,    99,   100,
     237,   414,    67,    68,    69,    70,   437,   222,    71,    72,
      73,    74,    75,   459,   488,   489,   490,   513,    76,    77,
      78,    79,    80,    95,   356,   474,    81,   475
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -380
static const yytype_int16 yypact[] =
{
     851,  -380,  -380,  -380,     5,   -52,   127,   -41,   -18,     7,
     -12,   567,  1116,  1116,    38,  -380,   851,    71,  1116,   110,
     112,  1116,    21,    47,  1116,  1116,   679,  1116,  1116,  1116,
       1,  -380,  1116,  1116,  -380,    46,  -380,  -380,  1116,  1116,
    -380,  -380,  -380,   111,  -380,   293,   212,  -380,  -380,  -380,
     258,   179,   363,   166,   341,    39,    96,   130,   155,     4,
    -380,  -380,    52,  -380,  -380,  -380,  -380,  -380,  -380,  -380,
    -380,  -380,  -380,  -380,  -380,  -380,  -380,  -380,  -380,  -380,
    -380,   203,   851,  -380,  -380,    50,   882,   127,   -55,   141,
    -380,  -380,  -380,   294,  -380,  -380,   175,    57,   175,    59,
    -380,  -380,  -380,    66,   184,  -380,  -380,   294,  -380,   300,
     303,  -380,  -380,  -380,   332,   348,   370,   168,   347,   185,
     209,   217,   244,    11,  -380,  -380,    62,   329,  -380,  -380,
    1116,   277,  1116,  -380,  1116,  1116,    85,   404,  -380,  -380,
    -380,  -380,  -380,   765,  -380,  -380,    36,  -380,   137,   968,
     224,  -380,  -380,   851,  -380,  -380,   999,  1116,   239,  -380,
    1116,   296,  -380,  -380,  -380,  -380,  -380,  -380,  -380,  -380,
    -380,  -380,  -380,  -380,  -380,  -380,  -380,  1116,  1116,  1116,
    1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,
    1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,
    1116,  1116,  -380,  -380,  1116,  -380,  -380,  -380,  -380,   307,
     278,   370,   181,   351,   327,   328,   333,   358,    75,  -380,
    -380,   343,   352,   294,  -380,    33,   355,  -380,   357,   -40,
     366,  -380,    91,  1116,   346,  -380,  1116,  -380,  -380,  -380,
     359,  -380,  -380,  -380,   -18,  -380,  -380,    82,  -380,  1116,
     361,  -380,  -380,  -380,  1116,  1116,  1116,  1116,  1116,  1116,
    1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,
    1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,  -380,
    -380,  1116,    94,   373,   123,   167,   171,  -380,  -380,   375,
      21,  -380,  -380,  -380,     1,  -380,  -380,  -380,  -380,  -380,
     202,  -380,   182,  -380,   218,  -380,  -380,  -380,  -380,  -380,
     332,   332,   348,   348,   348,   370,   370,   370,   370,   370,
     370,   168,   168,   168,   168,   347,   185,   209,   217,   244,
     371,  -380,    -4,   360,  1116,  1116,  1116,  1116,  1116,  1116,
    1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,  1116,
    1116,  1116,  1116,  -380,   388,  -380,   207,    90,  1116,  1116,
     379,  1116,  -380,   162,   248,  -380,  -380,   175,  -380,   390,
     208,   298,  -380,  -380,  -380,  -380,  -380,   332,   332,   348,
     348,   348,   370,   370,   370,   370,   370,   370,   168,   168,
     168,   168,   347,   185,   209,   217,   244,   377,  -380,   851,
    1116,   851,   391,   851,   369,  -380,  1085,  -380,  1116,  -380,
    -380,  1116,  1116,  1116,   432,  1116,   372,   210,   303,  -380,
     370,   370,   370,   370,   370,   181,   181,   181,   181,   351,
     327,   328,   333,   358,   380,  -380,   382,   381,   851,   401,
     378,   402,   211,  -380,  -380,   100,  -380,  -380,  -380,  -380,
    -380,   851,   405,  -380,  1116,   431,   219,  -380,   460,  -380,
    -380,   399,  -380,  -380,  -380,  -380,   267,  -380,  1116,   394,
     410,   851,  1116,  1116,   414,   851,   851,  -380,   851,   420,
     421,   272,   422,   851,  -380,   851,    73,  1116,    19,   460,
    -380,    21,   851,   273,  1116,  -380,  -380,  -380,   416,  -380,
     424,   427,   851,   851,   429,  -380,   430,  -380,  -380,  -380,
     -38,   419,  -380,   460,  -380,   458,  -380,   851,   426,   851,
    -380,  -380,   439,   440,   851,  -380,   851,   851,   444,    21,
    -380,   851,  -380,  -380,  -380,   449,   851,   851,  -380,  -380,
    -380,  -380
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -380,  -380,   128,  -380,  -380,     0,  -380,  -380,   226,  -380,
       6,  -380,     9,  -380,  -380,  -380,   -36,  -380,   262,  -380,
    -380,  -380,    30,    37,  -380,  -140,  -380,   -84,  -380,   -32,
    -380,  -161,    63,  -380,  -186,   173,  -380,  -184,   177,  -380,
    -178,   165,  -380,  -174,   186,  -380,  -179,   172,  -380,  -380,
    -380,  -380,  -380,  -380,  -380,  -109,  -328,  -380,   -22,   -10,
    -380,  -380,    22,   -19,  -380,  -380,  -380,  -380,  -380,   281,
     -93,    60,  -380,  -380,  -380,  -380,  -379,  -380,  -380,  -380,
    -380,  -380,  -380,  -380,    25,  -380,    53,  -380,  -380,  -380,
    -380,  -380,  -380,  -380,  -243,  -284,  -380,     2
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      41,   126,    82,   137,   370,   241,    83,   419,   101,   162,
     325,   136,    93,   326,   225,    94,    41,    86,   412,   146,
     327,   329,   434,   435,   177,   328,    41,   511,   143,   226,
      48,   321,   322,   323,   324,   359,   469,   526,   131,   281,
     296,   310,   311,    96,   360,   200,    48,   301,   138,   128,
     129,   207,   277,   202,   211,   133,    48,   235,   238,   413,
     242,   140,   141,   279,   144,   145,    98,   245,   306,   151,
     152,   248,   104,   251,   508,   154,   155,   201,   147,   512,
      26,    84,    41,   102,   278,   467,   287,   392,   254,    85,
     393,   103,   330,   223,   498,   331,   224,   394,   396,   312,
     313,   314,   395,   354,   206,   196,   292,   130,   388,   389,
     390,   391,    48,   281,   442,   518,   349,   355,   377,   378,
     282,   153,   284,   139,   285,   286,   208,   366,   203,   204,
       1,     2,     3,   239,   240,   243,   244,    87,   280,   281,
     132,    88,   246,    41,   497,   373,    15,   302,   350,   509,
     304,   362,   369,    41,   315,   316,   317,   318,   319,   320,
     441,   288,   281,   197,   399,   206,   355,   482,   363,   397,
     480,   281,   398,    48,   355,   298,   379,   380,   381,   134,
     156,   135,   157,    48,   355,   158,    89,   353,   186,   187,
     263,   264,   500,   401,   501,   198,    29,   199,    30,   506,
     281,   227,   481,   205,   336,   188,   189,   265,   266,   293,
      34,    90,    36,    37,   294,   307,   308,   309,   522,   523,
     337,   338,   447,   364,   228,   229,   230,   190,   191,   267,
     268,   382,   383,   384,   385,   386,   387,   402,   236,   371,
     535,   403,   339,   340,   281,   228,   229,   230,   281,   443,
     444,   273,   446,   247,   409,   163,   164,   181,   182,   281,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   405,   407,   110,   450,   176,   274,   439,   452,   408,
     471,   479,   275,   110,   440,   440,   276,   281,   440,   486,
     410,   110,   374,   375,   376,   281,   281,   463,   283,   464,
     334,   297,   465,   211,   420,   421,   422,   423,   424,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     449,   252,   253,   303,   417,   281,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   492,   178,   179,
     180,   176,   504,   517,   281,   484,   252,   253,   210,   440,
     281,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   156,   156,   160,   233,   176,   161,   234,   156,
     453,   249,   252,   253,   250,   281,   192,   193,   194,   195,
     305,   211,   269,   270,   271,   272,   341,   342,   343,   344,
     456,   332,   110,   345,   110,   346,   110,   110,   347,    41,
     348,    41,   466,    41,   425,   426,   427,   428,   183,   184,
     185,   110,   255,   256,   257,   260,   261,   262,   110,   110,
     351,   455,   110,   457,   357,   460,   258,   259,   352,    48,
     365,    48,   358,    48,   289,   290,   415,   416,    41,   110,
     211,   361,   400,   367,   404,   372,   411,   438,   445,   451,
     458,    41,   454,   461,   468,   472,   470,   473,   493,   281,
     476,   478,   477,   110,   483,   485,   110,   487,    48,   491,
     494,    41,   515,   413,   499,    41,    41,   510,    41,   502,
     503,    48,   505,    41,   520,    41,   519,   521,   524,   529,
     525,   448,    41,   496,   527,   110,   531,   206,   110,   533,
     534,    48,    41,    41,   538,    48,    48,   507,    48,   541,
     539,   110,   431,    48,   516,    48,   110,    41,   429,    41,
     406,   433,    48,   430,    41,   368,    41,    41,   536,   537,
     495,    41,    48,    48,   432,     0,    41,    41,   528,   530,
     110,   532,   514,   110,     0,     0,     0,    48,     0,    48,
       0,     0,     0,   540,    48,     0,    48,    48,   206,   206,
       0,    48,     0,     0,     0,     0,    48,    48,   105,     0,
       1,     2,     3,     0,     0,     0,     0,    87,     0,     0,
       0,    88,     0,    12,    13,     0,    15,     0,     0,     0,
       0,    18,     0,     0,     0,     0,   110,   418,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      24,    25,   418,   418,   110,     0,     0,     0,     0,     0,
     110,   110,     0,   110,     0,     0,    89,     0,     0,     0,
       0,    27,     0,     0,     0,    28,    29,     0,    30,     0,
       0,     0,     0,   106,     0,    32,    33,     0,     0,     0,
      34,    90,    36,    37,    38,    39,     0,     0,     0,     0,
       0,     0,   110,     0,     0,     0,     0,     0,   110,     0,
     110,     0,     0,   110,   110,   418,     0,   110,     0,     0,
       0,     0,     1,     2,     3,     4,     0,     0,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,    18,    19,    20,     0,    21,    22,     0,
       0,    23,     0,     0,     0,     0,   110,     0,     0,     0,
       0,     0,    24,    25,     0,     0,     0,     0,     0,     0,
     110,     0,     0,     0,   418,   110,     0,     0,    26,   142,
       0,     0,     0,    27,     0,     0,     0,    28,    29,   110,
      30,     0,     0,     0,     0,    31,   110,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,     1,     2,
       3,     4,     0,     0,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,     0,    18,
      19,    20,     0,    21,    22,     0,     0,    23,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    25,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    26,   291,     0,     0,     0,    27,
       0,     0,     0,    28,    29,     0,    30,     0,     0,     0,
       0,    31,     0,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,     1,     2,     3,     4,     0,     0,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,     0,     0,    18,    19,    20,     0,    21,
      22,     0,     0,    23,     0,     1,     2,     3,     0,     0,
       0,     0,    87,   209,    24,    25,    88,     0,    12,    13,
       0,    15,     0,     0,     0,     0,    18,     0,     0,     0,
      26,     0,     0,     0,     0,    27,     0,     0,     0,    28,
      29,     0,    30,     0,     0,    24,    25,    31,     0,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
       0,    89,     0,     0,     0,     0,    27,     0,     0,     0,
      28,    29,     0,    30,     0,     0,     0,     0,     0,     0,
      32,    33,     0,     0,     0,    34,    90,    36,    37,    38,
      39,     1,     2,     3,     0,     0,     0,     0,    87,     0,
       0,     0,    88,     0,    12,    13,     0,    15,     0,     0,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     1,     2,     3,     0,     0,     0,     0,    87,
       0,    24,    25,    88,     0,    12,    13,     0,    15,     0,
       0,     0,     0,    18,     0,     0,     0,    89,     0,     0,
       0,     0,    27,     0,     0,     0,    28,    29,     0,    30,
     295,     0,    24,    25,     0,     0,    32,    33,     0,     0,
       0,    34,    90,    36,    37,    38,    39,     0,    89,     0,
       0,     0,     0,    27,     0,     0,     0,    28,    29,   299,
      30,     0,     0,     0,     0,     0,     0,    32,    33,     0,
       0,     0,    34,    90,    36,    37,    38,    39,     1,     2,
       3,     0,     0,     0,     0,    87,     0,     0,     0,    88,
       0,    12,    13,     0,    15,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     1,
       2,     3,     0,     0,     0,     0,    87,     0,    24,    25,
      88,     0,    12,    13,     0,    15,     0,     0,     0,     0,
      18,     0,     0,     0,    89,     0,     0,     0,     0,    27,
       0,     0,     0,    28,    29,     0,    30,   462,     0,    24,
      25,     0,     0,    32,    33,     0,     0,     0,    34,    90,
      36,    37,    38,    39,     0,    89,     0,     0,     0,     0,
      27,     0,     0,     0,    28,    29,     0,    30,     0,     0,
       0,     0,     0,     0,    32,    33,     0,     0,     0,    34,
      90,    36,    37,    38,    39
};

static const yytype_int16 yycheck[] =
{
       0,    11,     0,    22,   247,    98,     1,   335,     1,    45,
     196,    21,     6,   197,    69,     6,    16,    69,    22,    29,
     198,   200,   350,   351,    46,   199,    26,     8,    26,    84,
       0,   192,   193,   194,   195,    75,   415,    75,    16,    77,
     149,   181,   182,    84,    84,    41,    16,   156,     1,    12,
      13,     1,    41,     1,    86,    18,    26,    93,     1,    63,
       1,    24,    25,     1,    27,    28,    84,     1,   177,    32,
      33,   107,    84,   109,     1,    38,    39,    73,    77,    60,
      59,    76,    82,    76,    73,   413,     1,   273,   110,    84,
     274,    84,   201,    87,   473,   204,    87,   275,   277,   183,
     184,   185,   276,    70,    82,    66,    70,    69,   269,   270,
     271,   272,    82,    77,   357,   494,    41,    84,   258,   259,
     130,    75,   132,    76,   134,   135,    76,   236,    76,    77,
       3,     4,     5,    76,    77,    76,    77,    10,    76,    77,
      69,    14,    76,   143,   472,   254,    19,   157,    73,    76,
     160,    60,    70,   153,   186,   187,   188,   189,   190,   191,
      70,    76,    77,    67,    70,   143,    84,   451,    77,   278,
      70,    77,   281,   143,    84,   153,   260,   261,   262,    69,
      69,    69,    71,   153,    84,    74,    59,   223,    22,    23,
      22,    23,   476,    70,   478,    65,    69,    42,    71,   483,
      77,    60,   445,     0,    23,    39,    40,    39,    40,    72,
      83,    84,    85,    86,    77,   178,   179,   180,   502,   503,
      39,    40,    60,   233,    83,    84,    85,    61,    62,    61,
      62,   263,   264,   265,   266,   267,   268,    70,    63,   249,
     524,    70,    61,    62,    77,    83,    84,    85,    77,   358,
     359,    66,   361,    69,    72,    43,    44,    78,    79,    77,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,   290,    70,    11,   367,    63,    67,    70,    70,    77,
      70,    70,    65,    21,    77,    77,    42,    77,    77,    70,
      72,    29,   255,   256,   257,    77,    77,   406,    21,   408,
      22,    77,   411,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
      72,    43,    44,    84,   334,    77,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    70,    80,    81,
      82,    63,    70,    70,    77,   454,    43,    44,    86,    77,
      77,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    69,    69,    71,    71,    63,    74,    74,    69,
      72,    71,    43,    44,    74,    77,    35,    36,    37,    38,
      84,   413,    35,    36,    37,    38,    35,    36,    37,    38,
     400,    84,   130,    66,   132,    67,   134,   135,    65,   399,
      42,   401,   412,   403,   341,   342,   343,   344,    45,    46,
      47,   149,    80,    81,    82,    45,    46,    47,   156,   157,
      77,   399,   160,   401,    69,   403,    78,    79,    76,   399,
      84,   401,    75,   403,    30,    31,    76,    77,   438,   177,
     472,    75,    69,    84,    69,    84,    75,    59,    69,    59,
      59,   451,    75,    84,    22,    75,    84,    76,   468,    77,
      59,    59,    84,   201,    59,    34,   204,     7,   438,    70,
      76,   471,   491,    63,    60,   475,   476,   487,   478,    59,
      59,   451,    60,   483,    60,   485,    70,    60,    59,    31,
      60,   363,   492,   471,    75,   233,    70,   475,   236,    60,
      60,   471,   502,   503,    60,   475,   476,   485,   478,    60,
     529,   249,   347,   483,   492,   485,   254,   517,   345,   519,
     294,   349,   492,   346,   524,   244,   526,   527,   526,   527,
     470,   531,   502,   503,   348,    -1,   536,   537,   513,   517,
     278,   519,   489,   281,    -1,    -1,    -1,   517,    -1,   519,
      -1,    -1,    -1,   531,   524,    -1,   526,   527,   536,   537,
      -1,   531,    -1,    -1,    -1,    -1,   536,   537,     1,    -1,
       3,     4,     5,    -1,    -1,    -1,    -1,    10,    -1,    -1,
      -1,    14,    -1,    16,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,   334,   335,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,   350,   351,   352,    -1,    -1,    -1,    -1,    -1,
     358,   359,    -1,   361,    -1,    -1,    59,    -1,    -1,    -1,
      -1,    64,    -1,    -1,    -1,    68,    69,    -1,    71,    -1,
      -1,    -1,    -1,    76,    -1,    78,    79,    -1,    -1,    -1,
      83,    84,    85,    86,    87,    88,    -1,    -1,    -1,    -1,
      -1,    -1,   400,    -1,    -1,    -1,    -1,    -1,   406,    -1,
     408,    -1,    -1,   411,   412,   413,    -1,   415,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,    -1,    -1,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,
      -1,    32,    -1,    -1,    -1,    -1,   454,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
     468,    -1,    -1,    -1,   472,   473,    -1,    -1,    59,    60,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    68,    69,   487,
      71,    -1,    -1,    -1,    -1,    76,   494,    78,    79,    -1,
      -1,    -1,    83,    84,    85,    86,    87,    88,     3,     4,
       5,     6,    -1,    -1,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    24,
      25,    26,    -1,    28,    29,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    59,    60,    -1,    -1,    -1,    64,
      -1,    -1,    -1,    68,    69,    -1,    71,    -1,    -1,    -1,
      -1,    76,    -1,    78,    79,    -1,    -1,    -1,    83,    84,
      85,    86,    87,    88,     3,     4,     5,     6,    -1,    -1,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    -1,    -1,    24,    25,    26,    -1,    28,
      29,    -1,    -1,    32,    -1,     3,     4,     5,    -1,    -1,
      -1,    -1,    10,    11,    43,    44,    14,    -1,    16,    17,
      -1,    19,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,
      59,    -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    68,
      69,    -1,    71,    -1,    -1,    43,    44,    76,    -1,    78,
      79,    -1,    -1,    -1,    83,    84,    85,    86,    87,    88,
      -1,    59,    -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,
      68,    69,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    -1,    -1,    -1,    83,    84,    85,    86,    87,
      88,     3,     4,     5,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    14,    -1,    16,    17,    -1,    19,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,    -1,    -1,    -1,    -1,    10,
      -1,    43,    44,    14,    -1,    16,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    -1,    -1,    -1,    59,    -1,    -1,
      -1,    -1,    64,    -1,    -1,    -1,    68,    69,    -1,    71,
      72,    -1,    43,    44,    -1,    -1,    78,    79,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    88,    -1,    59,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    68,    69,    70,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    -1,
      -1,    -1,    83,    84,    85,    86,    87,    88,     3,     4,
       5,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    14,
      -1,    16,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,    -1,    -1,    -1,    -1,    10,    -1,    43,    44,
      14,    -1,    16,    17,    -1,    19,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,    64,
      -1,    -1,    -1,    68,    69,    -1,    71,    72,    -1,    43,
      44,    -1,    -1,    78,    79,    -1,    -1,    -1,    83,    84,
      85,    86,    87,    88,    -1,    59,    -1,    -1,    -1,    -1,
      64,    -1,    -1,    -1,    68,    69,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    78,    79,    -1,    -1,    -1,    83,
      84,    85,    86,    87,    88
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    24,    25,
      26,    28,    29,    32,    43,    44,    59,    64,    68,    69,
      71,    76,    78,    79,    83,    84,    85,    86,    87,    88,
      90,    94,    95,   100,   102,   104,   108,   110,   111,   113,
     115,   117,   119,   122,   125,   128,   131,   134,   137,   140,
     143,   146,   150,   151,   152,   153,   156,   161,   162,   163,
     164,   167,   168,   169,   170,   171,   177,   178,   179,   180,
     181,   185,   186,     1,    76,    84,    69,    10,    14,    59,
      84,    93,    94,    99,   101,   182,    84,   154,    84,   157,
     158,     1,    76,    84,    84,     1,    76,    99,   101,   103,
     107,   109,   111,   112,   114,   116,   118,   120,   123,   126,
     129,   132,   135,   138,   141,   144,   148,   107,   112,   112,
      69,   151,    69,   112,    69,    69,   148,   152,     1,    76,
     112,   112,    60,   186,   112,   112,   148,    77,    96,    97,
      98,   112,   112,    75,   112,   112,    69,    71,    74,   105,
      71,    74,   105,    43,    44,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    63,   147,    80,    81,
      82,    78,    79,    45,    46,    47,    22,    23,    39,    40,
      61,    62,    35,    36,    37,    38,    66,    67,    65,    42,
      41,    73,     1,    76,    77,     0,   151,     1,    76,    11,
     107,   118,   121,   124,   127,   130,   133,   136,   139,   142,
     145,   149,   166,    99,   101,    69,    84,    60,    83,    84,
      85,    91,    92,    71,    74,   105,    63,   159,     1,    76,
      77,   159,     1,    76,    77,     1,    76,    69,   105,    71,
      74,   105,    43,    44,   147,    80,    81,    82,    78,    79,
      45,    46,    47,    22,    23,    39,    40,    61,    62,    35,
      36,    37,    38,    66,    67,    65,    42,    41,    73,     1,
      76,    77,   148,    21,   148,   148,   148,     1,    76,    30,
      31,    60,    70,    72,    77,    72,   144,    77,   151,    70,
     106,   144,   148,    84,   148,    84,   144,   112,   112,   112,
     114,   114,   116,   116,   116,   118,   118,   118,   118,   118,
     118,   120,   120,   120,   120,   123,   126,   129,   132,   135,
     144,   144,    84,   155,    22,   147,    23,    39,    40,    61,
      62,    35,    36,    37,    38,    66,    67,    65,    42,    41,
      73,    77,    76,   105,    70,    84,   183,    69,    75,    75,
      84,    75,    60,    77,   148,    84,   144,    84,   158,    70,
     183,   148,    84,   144,   112,   112,   112,   114,   114,   116,
     116,   116,   118,   118,   118,   118,   118,   118,   120,   120,
     120,   120,   123,   126,   129,   132,   135,   144,   144,    70,
      69,    70,    70,    70,    69,   152,    97,    70,    77,    72,
      72,    75,    22,    63,   160,    76,    77,   148,   107,   145,
     118,   118,   118,   118,   118,   121,   121,   121,   121,   124,
     127,   130,   133,   136,   145,   145,   148,   165,    59,    70,
      77,    70,   183,   144,   144,    69,   144,    60,    91,    72,
     159,    59,    70,    72,    75,   151,   148,   151,    59,   172,
     151,    84,    72,   144,   144,   144,   148,   145,    22,   165,
      84,    70,    75,    76,   184,   186,    59,    84,    59,    70,
      70,   183,   184,    59,   144,    34,    70,     7,   173,   174,
     175,    70,    70,   148,    76,   160,   151,   145,   165,    60,
     184,   184,    59,    59,    70,    60,   184,   151,     1,    76,
     148,     8,    60,   176,   175,   152,   151,    70,   165,    70,
      60,    60,   184,   184,    59,    60,    75,    75,   173,    31,
     151,    70,   151,    60,    60,   184,   186,   186,    60,   152,
     151,    60
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[2];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 299 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new NullNode(GLOBAL_DATA), 0, 1); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 300 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new BooleanNode(GLOBAL_DATA, true), 0, 1); ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 301 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new BooleanNode(GLOBAL_DATA, false), 0, 1); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 302 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeNumberNode(GLOBAL_DATA, (yyvsp[(1) - (1)].doubleValue)), 0, 1); ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 303 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new StringNode(GLOBAL_DATA, *(yyvsp[(1) - (1)].ident)), 0, 1); ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 304 "..\\lib\\Parse\\ecmascript.y"
    {
                                            size_t lastDIV = (yyvsp[(1) - (1)].ident)->name.rfind('/');
											const Identifier* pattern = new Identifier((yyvsp[(1) - (1)].ident)->name.c_str(), lastDIV);
											const Identifier* flags = new Identifier((yyvsp[(1) - (1)].ident)->name.c_str() + lastDIV + 1, (yyvsp[(1) - (1)].ident)->name.size() - lastDIV - 1);
                                            RegExpNode* node = new RegExpNode(GLOBAL_DATA, *pattern, *flags);
                                            setExceptionLocation(node, (yylsp[(1) - (1)]).first_column, (yylsp[(1) - (1)]).first_column + lastDIV, (yylsp[(1) - (1)]).first_column + lastDIV);
                                            (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, 0, 0);
											delete (yyvsp[(1) - (1)].ident);
										;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 316 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.propertyNode) = createNodeInfo<PropertyNode*>(new PropertyNode(GLOBAL_DATA, *(yyvsp[(1) - (3)].ident), (yyvsp[(3) - (3)].expressionNode).m_node, PropertyNode::Constant), (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 317 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.propertyNode) = createNodeInfo<PropertyNode*>(new PropertyNode(GLOBAL_DATA, *(yyvsp[(1) - (3)].ident), (yyvsp[(3) - (3)].expressionNode).m_node, PropertyNode::Constant), (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 318 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.propertyNode) = createNodeInfo<PropertyNode*>(new PropertyNode(GLOBAL_DATA, (yyvsp[(1) - (3)].doubleValue), (yyvsp[(3) - (3)].expressionNode).m_node, PropertyNode::Constant), (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 319 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.propertyNode) = createNodeInfo<PropertyNode*>(makeGetterOrSetterPropertyNode(GLOBAL_DATA, *(yyvsp[(1) - (7)].ident), *(yyvsp[(2) - (7)].ident), 0, (yyvsp[(6) - (7)].functionBodyNode)), ClosureFeature, 0); setStatementLocation((yyvsp[(6) - (7)].functionBodyNode), (yylsp[(5) - (7)]), (yylsp[(7) - (7)])); if (!(yyval.propertyNode).m_node) YYABORT; ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 321 "..\\lib\\Parse\\ecmascript.y"
    {
                                                                 (yyval.propertyNode) = createNodeInfo<PropertyNode*>(makeGetterOrSetterPropertyNode(GLOBAL_DATA, *(yyvsp[(1) - (8)].ident), *(yyvsp[(2) - (8)].ident), (yyvsp[(4) - (8)].parameterList).m_node.head, (yyvsp[(7) - (8)].functionBodyNode)), (yyvsp[(4) - (8)].parameterList).m_features | ClosureFeature, 0); 
                                                                 if ((yyvsp[(4) - (8)].parameterList).m_features & ArgumentsFeature)
                                                                     (yyvsp[(7) - (8)].functionBodyNode)->setUsesArguments(); 
                                                                 setStatementLocation((yyvsp[(7) - (8)].functionBodyNode), (yylsp[(6) - (8)]), (yylsp[(8) - (8)])); 
                                                                 if (!(yyval.propertyNode).m_node) 
                                                                     YYABORT; 
                                                             ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 332 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.propertyList).m_node.head = new PropertyListNode(GLOBAL_DATA, (yyvsp[(1) - (1)].propertyNode).m_node); 
                                          (yyval.propertyList).m_node.tail = (yyval.propertyList).m_node.head;
                                          (yyval.propertyList).m_features = (yyvsp[(1) - (1)].propertyNode).m_features;
                                          (yyval.propertyList).m_numConstants = (yyvsp[(1) - (1)].propertyNode).m_numConstants; ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 336 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.propertyList).m_node.head = (yyvsp[(1) - (3)].propertyList).m_node.head;
                                          (yyval.propertyList).m_node.tail = new PropertyListNode(GLOBAL_DATA, (yyvsp[(3) - (3)].propertyNode).m_node, (yyvsp[(1) - (3)].propertyList).m_node.tail);
                                          (yyval.propertyList).m_features = (yyvsp[(1) - (3)].propertyList).m_features | (yyvsp[(3) - (3)].propertyNode).m_features;
                                          (yyval.propertyList).m_numConstants = (yyvsp[(1) - (3)].propertyList).m_numConstants + (yyvsp[(3) - (3)].propertyNode).m_numConstants; ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 344 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ObjectLiteralNode(GLOBAL_DATA), 0, 0); ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 345 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ObjectLiteralNode(GLOBAL_DATA, (yyvsp[(2) - (3)].propertyList).m_node.head), (yyvsp[(2) - (3)].propertyList).m_features, (yyvsp[(2) - (3)].propertyList).m_numConstants); ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 347 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ObjectLiteralNode(GLOBAL_DATA, (yyvsp[(2) - (4)].propertyList).m_node.head), (yyvsp[(2) - (4)].propertyList).m_features, (yyvsp[(2) - (4)].propertyList).m_numConstants); ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 351 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ThisNode(GLOBAL_DATA), ThisFeature, 0); ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 354 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ResolveNode(GLOBAL_DATA, *(yyvsp[(1) - (1)].ident), (yylsp[(1) - (1)]).first_column), (*(yyvsp[(1) - (1)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0, 0); ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 355 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = (yyvsp[(2) - (3)].expressionNode); ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 359 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ArrayNode(GLOBAL_DATA, (yyvsp[(2) - (3)].intValue)), 0, (yyvsp[(2) - (3)].intValue) ? 1 : 0); ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 360 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ArrayNode(GLOBAL_DATA, 0, (yyvsp[(2) - (3)].elementList).m_node.head), (yyvsp[(2) - (3)].elementList).m_features, (yyvsp[(2) - (3)].elementList).m_numConstants); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 361 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ArrayNode(GLOBAL_DATA, (yyvsp[(4) - (5)].intValue), (yyvsp[(2) - (5)].elementList).m_node.head), (yyvsp[(2) - (5)].elementList).m_features, (yyvsp[(4) - (5)].intValue) ? (yyvsp[(2) - (5)].elementList).m_numConstants + 1 : (yyvsp[(2) - (5)].elementList).m_numConstants); ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 365 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.elementList).m_node.head = new ElementNode(GLOBAL_DATA, (yyvsp[(1) - (2)].intValue), (yyvsp[(2) - (2)].expressionNode).m_node);
                                          (yyval.elementList).m_node.tail = (yyval.elementList).m_node.head;
                                          (yyval.elementList).m_features = (yyvsp[(2) - (2)].expressionNode).m_features;
                                          (yyval.elementList).m_numConstants = (yyvsp[(2) - (2)].expressionNode).m_numConstants; ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 370 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.elementList).m_node.head = (yyvsp[(1) - (4)].elementList).m_node.head;
                                          (yyval.elementList).m_node.tail = new ElementNode(GLOBAL_DATA, (yyvsp[(3) - (4)].intValue), (yyvsp[(4) - (4)].expressionNode).m_node, (yyvsp[(1) - (4)].elementList).m_node.tail);
                                          (yyval.elementList).m_features = (yyvsp[(1) - (4)].elementList).m_features | (yyvsp[(4) - (4)].expressionNode).m_features;
                                          (yyval.elementList).m_numConstants = (yyvsp[(1) - (4)].elementList).m_numConstants + (yyvsp[(4) - (4)].expressionNode).m_numConstants; ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 377 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.intValue) = 0; ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 382 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.intValue) = 1; ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 383 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.intValue) = (yyvsp[(1) - (2)].intValue) + 1; ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 388 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>((yyvsp[(1) - (1)].funcExprNode).m_node, (yyvsp[(1) - (1)].funcExprNode).m_features, (yyvsp[(1) - (1)].funcExprNode).m_numConstants); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 389 "..\\lib\\Parse\\ecmascript.y"
    { BracketAccessorNode* node = new BracketAccessorNode(GLOBAL_DATA, (yyvsp[(1) - (4)].expressionNode).m_node, (yyvsp[(3) - (4)].expressionNode).m_node, (yyvsp[(3) - (4)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(1) - (4)]).first_column, (yylsp[(1) - (4)]).last_column, (yylsp[(4) - (4)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (4)].expressionNode).m_features | (yyvsp[(3) - (4)].expressionNode).m_features, (yyvsp[(1) - (4)].expressionNode).m_numConstants + (yyvsp[(3) - (4)].expressionNode).m_numConstants); 
                                        ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 393 "..\\lib\\Parse\\ecmascript.y"
    { DotAccessorNode* node = new DotAccessorNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, *(yyvsp[(3) - (3)].ident));
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(1) - (3)]).last_column, (yylsp[(3) - (3)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants);
                                        ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 397 "..\\lib\\Parse\\ecmascript.y"
    { NewExprNode* node = new NewExprNode(GLOBAL_DATA, (yyvsp[(2) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].argumentsNode).m_node);
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).last_column, (yylsp[(3) - (3)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(2) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].argumentsNode).m_features, (yyvsp[(2) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].argumentsNode).m_numConstants);
                                        ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 405 "..\\lib\\Parse\\ecmascript.y"
    { BracketAccessorNode* node = new BracketAccessorNode(GLOBAL_DATA, (yyvsp[(1) - (4)].expressionNode).m_node, (yyvsp[(3) - (4)].expressionNode).m_node, (yyvsp[(3) - (4)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(1) - (4)]).first_column, (yylsp[(1) - (4)]).last_column, (yylsp[(4) - (4)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (4)].expressionNode).m_features | (yyvsp[(3) - (4)].expressionNode).m_features, (yyvsp[(1) - (4)].expressionNode).m_numConstants + (yyvsp[(3) - (4)].expressionNode).m_numConstants); 
                                        ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 409 "..\\lib\\Parse\\ecmascript.y"
    { DotAccessorNode* node = new DotAccessorNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, *(yyvsp[(3) - (3)].ident));
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(1) - (3)]).last_column, (yylsp[(3) - (3)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants);
                                        ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 413 "..\\lib\\Parse\\ecmascript.y"
    { NewExprNode* node = new NewExprNode(GLOBAL_DATA, (yyvsp[(2) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].argumentsNode).m_node);
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).last_column, (yylsp[(3) - (3)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(2) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].argumentsNode).m_features, (yyvsp[(2) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].argumentsNode).m_numConstants);
                                        ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 421 "..\\lib\\Parse\\ecmascript.y"
    { NewExprNode* node = new NewExprNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node);
                                          setExceptionLocation(node, (yylsp[(1) - (2)]).first_column, (yylsp[(2) - (2)]).last_column, (yylsp[(2) - (2)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(2) - (2)].expressionNode).m_features, (yyvsp[(2) - (2)].expressionNode).m_numConstants); 
                                        ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 429 "..\\lib\\Parse\\ecmascript.y"
    { NewExprNode* node = new NewExprNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node);
                                          setExceptionLocation(node, (yylsp[(1) - (2)]).first_column, (yylsp[(2) - (2)]).last_column, (yylsp[(2) - (2)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(2) - (2)].expressionNode).m_features, (yyvsp[(2) - (2)].expressionNode).m_numConstants);
                                        ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 436 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = makeFunctionCallNode(GLOBAL_DATA, (yyvsp[(1) - (2)].expressionNode), (yyvsp[(2) - (2)].argumentsNode), (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(2) - (2)]).last_column); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 437 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = makeFunctionCallNode(GLOBAL_DATA, (yyvsp[(1) - (2)].expressionNode), (yyvsp[(2) - (2)].argumentsNode), (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(2) - (2)]).last_column); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 438 "..\\lib\\Parse\\ecmascript.y"
    { BracketAccessorNode* node = new BracketAccessorNode(GLOBAL_DATA, (yyvsp[(1) - (4)].expressionNode).m_node, (yyvsp[(3) - (4)].expressionNode).m_node, (yyvsp[(3) - (4)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(1) - (4)]).first_column, (yylsp[(1) - (4)]).last_column, (yylsp[(4) - (4)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (4)].expressionNode).m_features | (yyvsp[(3) - (4)].expressionNode).m_features, (yyvsp[(1) - (4)].expressionNode).m_numConstants + (yyvsp[(3) - (4)].expressionNode).m_numConstants); 
                                        ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 442 "..\\lib\\Parse\\ecmascript.y"
    { DotAccessorNode* node = new DotAccessorNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, *(yyvsp[(3) - (3)].ident));
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(1) - (3)]).last_column, (yylsp[(3) - (3)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 448 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = makeFunctionCallNode(GLOBAL_DATA, (yyvsp[(1) - (2)].expressionNode), (yyvsp[(2) - (2)].argumentsNode), (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(2) - (2)]).last_column); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 449 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = makeFunctionCallNode(GLOBAL_DATA, (yyvsp[(1) - (2)].expressionNode), (yyvsp[(2) - (2)].argumentsNode), (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(2) - (2)]).last_column); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 450 "..\\lib\\Parse\\ecmascript.y"
    { BracketAccessorNode* node = new BracketAccessorNode(GLOBAL_DATA, (yyvsp[(1) - (4)].expressionNode).m_node, (yyvsp[(3) - (4)].expressionNode).m_node, (yyvsp[(3) - (4)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(1) - (4)]).first_column, (yylsp[(1) - (4)]).last_column, (yylsp[(4) - (4)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (4)].expressionNode).m_features | (yyvsp[(3) - (4)].expressionNode).m_features, (yyvsp[(1) - (4)].expressionNode).m_numConstants + (yyvsp[(3) - (4)].expressionNode).m_numConstants); 
                                        ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 454 "..\\lib\\Parse\\ecmascript.y"
    { DotAccessorNode* node = new DotAccessorNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, *(yyvsp[(3) - (3)].ident));
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(1) - (3)]).last_column, (yylsp[(3) - (3)]).last_column);
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants); 
                                        ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 461 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.argumentsNode) = createNodeInfo<ArgumentsNode*>(new ArgumentsNode(GLOBAL_DATA), 0, 0); ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 462 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.argumentsNode) = createNodeInfo<ArgumentsNode*>(new ArgumentsNode(GLOBAL_DATA, (yyvsp[(2) - (3)].argumentList).m_node.head), (yyvsp[(2) - (3)].argumentList).m_features, (yyvsp[(2) - (3)].argumentList).m_numConstants); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 466 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.argumentList).m_node.head = new ArgumentListNode(GLOBAL_DATA, (yyvsp[(1) - (1)].expressionNode).m_node);
                                          (yyval.argumentList).m_node.tail = (yyval.argumentList).m_node.head;
                                          (yyval.argumentList).m_features = (yyvsp[(1) - (1)].expressionNode).m_features;
                                          (yyval.argumentList).m_numConstants = (yyvsp[(1) - (1)].expressionNode).m_numConstants; ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 470 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.argumentList).m_node.head = (yyvsp[(1) - (3)].argumentList).m_node.head;
                                          (yyval.argumentList).m_node.tail = new ArgumentListNode(GLOBAL_DATA, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(1) - (3)].argumentList).m_node.tail);
                                          (yyval.argumentList).m_features = (yyvsp[(1) - (3)].argumentList).m_features | (yyvsp[(3) - (3)].expressionNode).m_features;
                                          (yyval.argumentList).m_numConstants = (yyvsp[(1) - (3)].argumentList).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants; ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 488 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makePostfixNode(GLOBAL_DATA, (yyvsp[(1) - (2)].expressionNode).m_node, op_plusplus, (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(2) - (2)]).last_column), (yyvsp[(1) - (2)].expressionNode).m_features | AssignFeature, (yyvsp[(1) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 489 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makePostfixNode(GLOBAL_DATA, (yyvsp[(1) - (2)].expressionNode).m_node, op_minusminus, (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(2) - (2)]).last_column), (yyvsp[(1) - (2)].expressionNode).m_features | AssignFeature, (yyvsp[(1) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 494 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makePostfixNode(GLOBAL_DATA, (yyvsp[(1) - (2)].expressionNode).m_node, op_plusplus, (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(2) - (2)]).last_column), (yyvsp[(1) - (2)].expressionNode).m_features | AssignFeature, (yyvsp[(1) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 495 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makePostfixNode(GLOBAL_DATA, (yyvsp[(1) - (2)].expressionNode).m_node, op_minusminus, (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(2) - (2)]).last_column), (yyvsp[(1) - (2)].expressionNode).m_features | AssignFeature, (yyvsp[(1) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 499 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeDeleteNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node, (yylsp[(1) - (2)]).first_column, (yylsp[(2) - (2)]).last_column, (yylsp[(2) - (2)]).last_column), (yyvsp[(2) - (2)].expressionNode).m_features, (yyvsp[(2) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 500 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new VoidNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node), (yyvsp[(2) - (2)].expressionNode).m_features, (yyvsp[(2) - (2)].expressionNode).m_numConstants + 1); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 501 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeTypeOfNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node), (yyvsp[(2) - (2)].expressionNode).m_features, (yyvsp[(2) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 502 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makePrefixNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node, op_plusplus, (yylsp[(1) - (2)]).first_column, (yylsp[(2) - (2)]).first_column + 1, (yylsp[(2) - (2)]).last_column), (yyvsp[(2) - (2)].expressionNode).m_features | AssignFeature, (yyvsp[(2) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 503 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makePrefixNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node, op_plusplus, (yylsp[(1) - (2)]).first_column, (yylsp[(2) - (2)]).first_column + 1, (yylsp[(2) - (2)]).last_column), (yyvsp[(2) - (2)].expressionNode).m_features | AssignFeature, (yyvsp[(2) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 504 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makePrefixNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node, op_minusminus, (yylsp[(1) - (2)]).first_column, (yylsp[(2) - (2)]).first_column + 1, (yylsp[(2) - (2)]).last_column), (yyvsp[(2) - (2)].expressionNode).m_features | AssignFeature, (yyvsp[(2) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 505 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makePrefixNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node, op_minusminus, (yylsp[(1) - (2)]).first_column, (yylsp[(2) - (2)]).first_column + 1, (yylsp[(2) - (2)]).last_column), (yyvsp[(2) - (2)].expressionNode).m_features | AssignFeature, (yyvsp[(2) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 506 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new UnaryPlusNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node), (yyvsp[(2) - (2)].expressionNode).m_features, (yyvsp[(2) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 507 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeNegateNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node), (yyvsp[(2) - (2)].expressionNode).m_features, (yyvsp[(2) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 508 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeBitwiseNotNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node), (yyvsp[(2) - (2)].expressionNode).m_features, (yyvsp[(2) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 509 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LogicalNotNode(GLOBAL_DATA, (yyvsp[(2) - (2)].expressionNode).m_node), (yyvsp[(2) - (2)].expressionNode).m_features, (yyvsp[(2) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 523 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeMultNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 524 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeDivNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 525 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ModNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 531 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeMultNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 533 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeDivNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 535 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ModNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 540 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeAddNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 541 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeSubNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 547 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeAddNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 549 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeSubNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 554 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeLeftShiftNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 555 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeRightShiftNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 556 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new UnsignedRightShiftNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 561 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeLeftShiftNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 562 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeRightShiftNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 563 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new UnsignedRightShiftNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 568 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LessNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 569 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new GreaterNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 570 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LessEqNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 571 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new GreaterEqNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 572 "..\\lib\\Parse\\ecmascript.y"
    { InstanceOfNode* node = new InstanceOfNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(3) - (3)]).first_column, (yylsp[(3) - (3)]).last_column);  
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 575 "..\\lib\\Parse\\ecmascript.y"
    { InNode* node = new InNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(3) - (3)]).first_column, (yylsp[(3) - (3)]).last_column);  
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 582 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LessNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 583 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new GreaterNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 584 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LessEqNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 585 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new GreaterEqNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 587 "..\\lib\\Parse\\ecmascript.y"
    { InstanceOfNode* node = new InstanceOfNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(3) - (3)]).first_column, (yylsp[(3) - (3)]).last_column);  
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 594 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LessNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 595 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new GreaterNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 596 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LessEqNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 597 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new GreaterEqNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 599 "..\\lib\\Parse\\ecmascript.y"
    { InstanceOfNode* node = new InstanceOfNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(3) - (3)]).first_column, (yylsp[(3) - (3)]).last_column);  
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 603 "..\\lib\\Parse\\ecmascript.y"
    { InNode* node = new InNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(3) - (3)]).first_column, (yylsp[(3) - (3)]).last_column);  
                                          (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(node, (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 610 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new EqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 611 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new NotEqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 612 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new StrictEqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 613 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new NotStrictEqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 619 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new EqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 621 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new NotEqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 623 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new StrictEqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 625 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new NotStrictEqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 631 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new EqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 632 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new NotEqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 634 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new StrictEqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 636 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new NotStrictEqualNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 641 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new BitAndNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 647 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new BitAndNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 652 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new BitAndNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 657 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new BitXOrNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 663 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new BitXOrNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 669 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new BitXOrNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 674 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new BitOrNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 680 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new BitOrNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 686 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new BitOrNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 691 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, op_and), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 697 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, op_and), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 703 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, op_and), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 708 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, op_or), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 714 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, op_or), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 719 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new LogicalOpNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node, op_or), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 725 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ConditionalNode(GLOBAL_DATA, (yyvsp[(1) - (5)].expressionNode).m_node, (yyvsp[(3) - (5)].expressionNode).m_node, (yyvsp[(5) - (5)].expressionNode).m_node), (yyvsp[(1) - (5)].expressionNode).m_features | (yyvsp[(3) - (5)].expressionNode).m_features | (yyvsp[(5) - (5)].expressionNode).m_features, (yyvsp[(1) - (5)].expressionNode).m_numConstants + (yyvsp[(3) - (5)].expressionNode).m_numConstants + (yyvsp[(5) - (5)].expressionNode).m_numConstants); ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 731 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ConditionalNode(GLOBAL_DATA, (yyvsp[(1) - (5)].expressionNode).m_node, (yyvsp[(3) - (5)].expressionNode).m_node, (yyvsp[(5) - (5)].expressionNode).m_node), (yyvsp[(1) - (5)].expressionNode).m_features | (yyvsp[(3) - (5)].expressionNode).m_features | (yyvsp[(5) - (5)].expressionNode).m_features, (yyvsp[(1) - (5)].expressionNode).m_numConstants + (yyvsp[(3) - (5)].expressionNode).m_numConstants + (yyvsp[(5) - (5)].expressionNode).m_numConstants); ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 737 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(new ConditionalNode(GLOBAL_DATA, (yyvsp[(1) - (5)].expressionNode).m_node, (yyvsp[(3) - (5)].expressionNode).m_node, (yyvsp[(5) - (5)].expressionNode).m_node), (yyvsp[(1) - (5)].expressionNode).m_features | (yyvsp[(3) - (5)].expressionNode).m_features | (yyvsp[(5) - (5)].expressionNode).m_features, (yyvsp[(1) - (5)].expressionNode).m_numConstants + (yyvsp[(3) - (5)].expressionNode).m_numConstants + (yyvsp[(5) - (5)].expressionNode).m_numConstants); ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 743 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeAssignNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(1) - (3)].expressionNode).m_features & AssignFeature, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature, 
                                                                                                     (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).first_column + 1, (yylsp[(3) - (3)]).last_column), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features | AssignFeature, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); 
                                        ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 751 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeAssignNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(1) - (3)].expressionNode).m_features & AssignFeature, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature, 
                                                                                                     (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).first_column + 1, (yylsp[(3) - (3)]).last_column), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features | AssignFeature, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants);
                                        ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 759 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(makeAssignNode(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].expressionNode).m_node, (yyvsp[(1) - (3)].expressionNode).m_features & AssignFeature, (yyvsp[(3) - (3)].expressionNode).m_features & AssignFeature,
                                                                                                     (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).first_column + 1, (yylsp[(3) - (3)]).last_column), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features | AssignFeature, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); 
                                        ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 765 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_eq; ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 766 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_pluseq; ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 767 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_minuseq; ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 768 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_multeq; ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 769 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_diveq; ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 770 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_lshift; ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 771 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_rshift; ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 772 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_urshift; ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 773 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_andeq; ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 774 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_xoreq; ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 775 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_oreq; ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 776 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.op) = op_modeq; ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 781 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(combineCommaNodes(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 786 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(combineCommaNodes(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 791 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(combineCommaNodes(GLOBAL_DATA, (yyvsp[(1) - (3)].expressionNode).m_node, (yyvsp[(3) - (3)].expressionNode).m_node), (yyvsp[(1) - (3)].expressionNode).m_features | (yyvsp[(3) - (3)].expressionNode).m_features, (yyvsp[(1) - (3)].expressionNode).m_numConstants + (yyvsp[(3) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 815 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new BlockNode(GLOBAL_DATA, 0), 0, 0, 0, 0);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 817 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new BlockNode(GLOBAL_DATA, (yyvsp[(2) - (3)].sourceElements).m_node), (yyvsp[(2) - (3)].sourceElements).m_varDeclarations, (yyvsp[(2) - (3)].sourceElements).m_funcDeclarations, (yyvsp[(2) - (3)].sourceElements).m_features, (yyvsp[(2) - (3)].sourceElements).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 822 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(makeVarStatementNode(GLOBAL_DATA, (yyvsp[(2) - (3)].varDeclList).m_node), (yyvsp[(2) - (3)].varDeclList).m_varDeclarations, (yyvsp[(2) - (3)].varDeclList).m_funcDeclarations, (yyvsp[(2) - (3)].varDeclList).m_features, (yyvsp[(2) - (3)].varDeclList).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 824 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(makeVarStatementNode(GLOBAL_DATA, (yyvsp[(2) - (3)].varDeclList).m_node), (yyvsp[(2) - (3)].varDeclList).m_varDeclarations, (yyvsp[(2) - (3)].varDeclList).m_funcDeclarations, (yyvsp[(2) - (3)].varDeclList).m_features, (yyvsp[(2) - (3)].varDeclList).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(2) - (3)]));
                                          AUTO_SEMICOLON; ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 830 "..\\lib\\Parse\\ecmascript.y"
    { ResolveNode* node = new ResolveNode(GLOBAL_DATA, *(yyvsp[(1) - (1)].ident), (yylsp[(1) - (1)]).first_column);
										  (yyval.varDeclList).m_node = node;
                                          (yyval.varDeclList).m_varDeclarations = new ParserArenaData<DeclarationStacks::VarStack>;
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyval.varDeclList).m_varDeclarations, *(yyvsp[(1) - (1)].ident), 0);
                                          (yyval.varDeclList).m_funcDeclarations = 0;
                                          (yyval.varDeclList).m_features = (*(yyvsp[(1) - (1)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0;
                                          (yyval.varDeclList).m_numConstants = 0;
                                        ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 838 "..\\lib\\Parse\\ecmascript.y"
    { AssignResolveNode* node = new AssignResolveNode(GLOBAL_DATA, *(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].expressionNode).m_node, (yyvsp[(2) - (2)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(1) - (2)]).first_column, (yylsp[(2) - (2)]).first_column + 1, (yylsp[(2) - (2)]).last_column);
                                          (yyval.varDeclList).m_node = node;
                                          (yyval.varDeclList).m_varDeclarations = new ParserArenaData<DeclarationStacks::VarStack>;
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyval.varDeclList).m_varDeclarations, *(yyvsp[(1) - (2)].ident), DeclarationStacks::HasInitializer);
                                          (yyval.varDeclList).m_funcDeclarations = 0;
                                          (yyval.varDeclList).m_features = ((*(yyvsp[(1) - (2)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | (yyvsp[(2) - (2)].expressionNode).m_features;
                                          (yyval.varDeclList).m_numConstants = (yyvsp[(2) - (2)].expressionNode).m_numConstants;
                                        ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 848 "..\\lib\\Parse\\ecmascript.y"
    { /* eddid add ResolveNode node to fix "var a,b,c=2; b lost" issue */
										  ResolveNode* node = new ResolveNode(GLOBAL_DATA, *(yyvsp[(3) - (3)].ident), (yylsp[(3) - (3)]).first_column);
                                          (yyval.varDeclList).m_node = combineCommaNodes(GLOBAL_DATA, (yyvsp[(1) - (3)].varDeclList).m_node, node);
                                          (yyval.varDeclList).m_varDeclarations = (yyvsp[(1) - (3)].varDeclList).m_varDeclarations;
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyval.varDeclList).m_varDeclarations, *(yyvsp[(3) - (3)].ident), 0);
                                          (yyval.varDeclList).m_funcDeclarations = 0;
                                          (yyval.varDeclList).m_features = (yyvsp[(1) - (3)].varDeclList).m_features | ((*(yyvsp[(3) - (3)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0);
                                          (yyval.varDeclList).m_numConstants = (yyvsp[(1) - (3)].varDeclList).m_numConstants;
                                        ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 858 "..\\lib\\Parse\\ecmascript.y"
    { AssignResolveNode* node = new AssignResolveNode(GLOBAL_DATA, *(yyvsp[(3) - (4)].ident), (yyvsp[(4) - (4)].expressionNode).m_node, (yyvsp[(4) - (4)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(3) - (4)]).first_column, (yylsp[(4) - (4)]).first_column + 1, (yylsp[(4) - (4)]).last_column);
                                          (yyval.varDeclList).m_node = combineCommaNodes(GLOBAL_DATA, (yyvsp[(1) - (4)].varDeclList).m_node, node);
                                          (yyval.varDeclList).m_varDeclarations = (yyvsp[(1) - (4)].varDeclList).m_varDeclarations;
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyval.varDeclList).m_varDeclarations, *(yyvsp[(3) - (4)].ident), DeclarationStacks::HasInitializer);
                                          (yyval.varDeclList).m_funcDeclarations = 0;
                                          (yyval.varDeclList).m_features = (yyvsp[(1) - (4)].varDeclList).m_features | ((*(yyvsp[(3) - (4)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | (yyvsp[(4) - (4)].expressionNode).m_features;
                                          (yyval.varDeclList).m_numConstants = (yyvsp[(1) - (4)].varDeclList).m_numConstants + (yyvsp[(4) - (4)].expressionNode).m_numConstants;
                                        ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 870 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.varDeclList).m_node = 0;
                                          (yyval.varDeclList).m_varDeclarations = new ParserArenaData<DeclarationStacks::VarStack>;
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyval.varDeclList).m_varDeclarations, *(yyvsp[(1) - (1)].ident), 0);
                                          (yyval.varDeclList).m_funcDeclarations = 0;
                                          (yyval.varDeclList).m_features = (*(yyvsp[(1) - (1)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0;
                                          (yyval.varDeclList).m_numConstants = 0;
                                        ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 877 "..\\lib\\Parse\\ecmascript.y"
    { AssignResolveNode* node = new AssignResolveNode(GLOBAL_DATA, *(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].expressionNode).m_node, (yyvsp[(2) - (2)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(1) - (2)]).first_column, (yylsp[(2) - (2)]).first_column + 1, (yylsp[(2) - (2)]).last_column);
                                          (yyval.varDeclList).m_node = node;
                                          (yyval.varDeclList).m_varDeclarations = new ParserArenaData<DeclarationStacks::VarStack>;
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyval.varDeclList).m_varDeclarations, *(yyvsp[(1) - (2)].ident), DeclarationStacks::HasInitializer);
                                          (yyval.varDeclList).m_funcDeclarations = 0;
                                          (yyval.varDeclList).m_features = ((*(yyvsp[(1) - (2)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | (yyvsp[(2) - (2)].expressionNode).m_features;
                                          (yyval.varDeclList).m_numConstants = (yyvsp[(2) - (2)].expressionNode).m_numConstants;
                                        ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 887 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.varDeclList).m_node = (yyvsp[(1) - (3)].varDeclList).m_node;
                                          (yyval.varDeclList).m_varDeclarations = (yyvsp[(1) - (3)].varDeclList).m_varDeclarations;
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyval.varDeclList).m_varDeclarations, *(yyvsp[(3) - (3)].ident), 0);
                                          (yyval.varDeclList).m_funcDeclarations = 0;
                                          (yyval.varDeclList).m_features = (yyvsp[(1) - (3)].varDeclList).m_features | ((*(yyvsp[(3) - (3)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0);
                                          (yyval.varDeclList).m_numConstants = (yyvsp[(1) - (3)].varDeclList).m_numConstants;
                                        ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 895 "..\\lib\\Parse\\ecmascript.y"
    { AssignResolveNode* node = new AssignResolveNode(GLOBAL_DATA, *(yyvsp[(3) - (4)].ident), (yyvsp[(4) - (4)].expressionNode).m_node, (yyvsp[(4) - (4)].expressionNode).m_features & AssignFeature);
                                          setExceptionLocation(node, (yylsp[(3) - (4)]).first_column, (yylsp[(4) - (4)]).first_column + 1, (yylsp[(4) - (4)]).last_column);
                                          (yyval.varDeclList).m_node = combineCommaNodes(GLOBAL_DATA, (yyvsp[(1) - (4)].varDeclList).m_node, node);
                                          (yyval.varDeclList).m_varDeclarations = (yyvsp[(1) - (4)].varDeclList).m_varDeclarations;
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyval.varDeclList).m_varDeclarations, *(yyvsp[(3) - (4)].ident), DeclarationStacks::HasInitializer);
                                          (yyval.varDeclList).m_funcDeclarations = 0;
                                          (yyval.varDeclList).m_features = (yyvsp[(1) - (4)].varDeclList).m_features | ((*(yyvsp[(3) - (4)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | (yyvsp[(4) - (4)].expressionNode).m_features;
                                          (yyval.varDeclList).m_numConstants = (yyvsp[(1) - (4)].varDeclList).m_numConstants + (yyvsp[(4) - (4)].expressionNode).m_numConstants;
                                        ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 907 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new ConstStatementNode(GLOBAL_DATA, (yyvsp[(2) - (3)].constDeclList).m_node.head), (yyvsp[(2) - (3)].constDeclList).m_varDeclarations, (yyvsp[(2) - (3)].constDeclList).m_funcDeclarations, (yyvsp[(2) - (3)].constDeclList).m_features, (yyvsp[(2) - (3)].constDeclList).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 910 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new ConstStatementNode(GLOBAL_DATA, (yyvsp[(2) - (3)].constDeclList).m_node.head), (yyvsp[(2) - (3)].constDeclList).m_varDeclarations, (yyvsp[(2) - (3)].constDeclList).m_funcDeclarations, (yyvsp[(2) - (3)].constDeclList).m_features, (yyvsp[(2) - (3)].constDeclList).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 915 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.constDeclList).m_node.head = (yyvsp[(1) - (1)].constDeclNode).m_node;
                                          (yyval.constDeclList).m_node.tail = (yyval.constDeclList).m_node.head;
                                          (yyval.constDeclList).m_varDeclarations = new ParserArenaData<DeclarationStacks::VarStack>;
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyval.constDeclList).m_varDeclarations, (yyvsp[(1) - (1)].constDeclNode).m_node);
                                          (yyval.constDeclList).m_funcDeclarations = 0; 
                                          (yyval.constDeclList).m_features = (yyvsp[(1) - (1)].constDeclNode).m_features;
                                          (yyval.constDeclList).m_numConstants = (yyvsp[(1) - (1)].constDeclNode).m_numConstants;
    ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 924 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.constDeclList).m_node.head = (yyvsp[(1) - (3)].constDeclList).m_node.head;
                                          (yyvsp[(1) - (3)].constDeclList).m_node.tail->setNext((yyvsp[(3) - (3)].constDeclNode).m_node);
                                          (yyval.constDeclList).m_node.tail = (yyvsp[(3) - (3)].constDeclNode).m_node;
                                          (yyval.constDeclList).m_varDeclarations = (yyvsp[(1) - (3)].constDeclList).m_varDeclarations;
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyval.constDeclList).m_varDeclarations, (yyvsp[(3) - (3)].constDeclNode).m_node);
                                          (yyval.constDeclList).m_funcDeclarations = 0;
                                          (yyval.constDeclList).m_features = (yyvsp[(1) - (3)].constDeclList).m_features | (yyvsp[(3) - (3)].constDeclNode).m_features;
                                          (yyval.constDeclList).m_numConstants = (yyvsp[(1) - (3)].constDeclList).m_numConstants + (yyvsp[(3) - (3)].constDeclNode).m_numConstants; ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 935 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.constDeclNode) = createNodeInfo<ConstDeclNode*>(new ConstDeclNode(GLOBAL_DATA, *(yyvsp[(1) - (1)].ident), 0), (*(yyvsp[(1) - (1)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0, 0); ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 936 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.constDeclNode) = createNodeInfo<ConstDeclNode*>(new ConstDeclNode(GLOBAL_DATA, *(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].expressionNode).m_node), ((*(yyvsp[(1) - (2)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | (yyvsp[(2) - (2)].expressionNode).m_features, (yyvsp[(2) - (2)].expressionNode).m_numConstants); ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 940 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = (yyvsp[(2) - (2)].expressionNode); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 944 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = (yyvsp[(2) - (2)].expressionNode); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 948 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new EmptyStatementNode(GLOBAL_DATA), 0, 0, 0, 0); ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 952 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new ExprStatementNode(GLOBAL_DATA, (yyvsp[(1) - (2)].expressionNode).m_node), 0, 0, (yyvsp[(1) - (2)].expressionNode).m_features, (yyvsp[(1) - (2)].expressionNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 954 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new ExprStatementNode(GLOBAL_DATA, (yyvsp[(1) - (2)].expressionNode).m_node), 0, 0, (yyvsp[(1) - (2)].expressionNode).m_features, (yyvsp[(1) - (2)].expressionNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 960 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new IfNode(GLOBAL_DATA, (yyvsp[(3) - (5)].expressionNode).m_node, (yyvsp[(5) - (5)].statementNode).m_node), (yyvsp[(5) - (5)].statementNode).m_varDeclarations, (yyvsp[(5) - (5)].statementNode).m_funcDeclarations, (yyvsp[(3) - (5)].expressionNode).m_features | (yyvsp[(5) - (5)].statementNode).m_features, (yyvsp[(3) - (5)].expressionNode).m_numConstants + (yyvsp[(5) - (5)].statementNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 963 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new IfElseNode(GLOBAL_DATA, (yyvsp[(3) - (7)].expressionNode).m_node, (yyvsp[(5) - (7)].statementNode).m_node, (yyvsp[(7) - (7)].statementNode).m_node), 
                                                                                         mergeDeclarationLists((yyvsp[(5) - (7)].statementNode).m_varDeclarations, (yyvsp[(7) - (7)].statementNode).m_varDeclarations),
                                                                                         mergeDeclarationLists((yyvsp[(5) - (7)].statementNode).m_funcDeclarations, (yyvsp[(7) - (7)].statementNode).m_funcDeclarations),
                                                                                         (yyvsp[(3) - (7)].expressionNode).m_features | (yyvsp[(5) - (7)].statementNode).m_features | (yyvsp[(7) - (7)].statementNode).m_features,
                                                                                         (yyvsp[(3) - (7)].expressionNode).m_numConstants + (yyvsp[(5) - (7)].statementNode).m_numConstants + (yyvsp[(7) - (7)].statementNode).m_numConstants); 
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (7)]), (yylsp[(4) - (7)])); ;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 972 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new DoWhileNode(GLOBAL_DATA, (yyvsp[(2) - (7)].statementNode).m_node, (yyvsp[(5) - (7)].expressionNode).m_node), (yyvsp[(2) - (7)].statementNode).m_varDeclarations, (yyvsp[(2) - (7)].statementNode).m_funcDeclarations, (yyvsp[(2) - (7)].statementNode).m_features | (yyvsp[(5) - (7)].expressionNode).m_features, (yyvsp[(2) - (7)].statementNode).m_numConstants + (yyvsp[(5) - (7)].expressionNode).m_numConstants);
                                             setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (7)]), (yylsp[(3) - (7)])); ;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 974 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new DoWhileNode(GLOBAL_DATA, (yyvsp[(2) - (7)].statementNode).m_node, (yyvsp[(5) - (7)].expressionNode).m_node), (yyvsp[(2) - (7)].statementNode).m_varDeclarations, (yyvsp[(2) - (7)].statementNode).m_funcDeclarations, (yyvsp[(2) - (7)].statementNode).m_features | (yyvsp[(5) - (7)].expressionNode).m_features, (yyvsp[(2) - (7)].statementNode).m_numConstants + (yyvsp[(5) - (7)].expressionNode).m_numConstants);
                                             setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (7)]), (yylsp[(3) - (7)])); ;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 976 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new WhileNode(GLOBAL_DATA, (yyvsp[(3) - (5)].expressionNode).m_node, (yyvsp[(5) - (5)].statementNode).m_node), (yyvsp[(5) - (5)].statementNode).m_varDeclarations, (yyvsp[(5) - (5)].statementNode).m_funcDeclarations, (yyvsp[(3) - (5)].expressionNode).m_features | (yyvsp[(5) - (5)].statementNode).m_features, (yyvsp[(3) - (5)].expressionNode).m_numConstants + (yyvsp[(5) - (5)].statementNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 979 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new ForNode(GLOBAL_DATA, (yyvsp[(3) - (9)].expressionNode).m_node, (yyvsp[(5) - (9)].expressionNode).m_node, (yyvsp[(7) - (9)].expressionNode).m_node, (yyvsp[(9) - (9)].statementNode).m_node, false), (yyvsp[(9) - (9)].statementNode).m_varDeclarations, (yyvsp[(9) - (9)].statementNode).m_funcDeclarations, 
                                                                                         (yyvsp[(3) - (9)].expressionNode).m_features | (yyvsp[(5) - (9)].expressionNode).m_features | (yyvsp[(7) - (9)].expressionNode).m_features | (yyvsp[(9) - (9)].statementNode).m_features,
                                                                                         (yyvsp[(3) - (9)].expressionNode).m_numConstants + (yyvsp[(5) - (9)].expressionNode).m_numConstants + (yyvsp[(7) - (9)].expressionNode).m_numConstants + (yyvsp[(9) - (9)].statementNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (9)]), (yylsp[(8) - (9)])); 
                                        ;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 985 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new ForNode(GLOBAL_DATA, (yyvsp[(4) - (10)].varDeclList).m_node, (yyvsp[(6) - (10)].expressionNode).m_node, (yyvsp[(8) - (10)].expressionNode).m_node, (yyvsp[(10) - (10)].statementNode).m_node, true),
                                                                                         mergeDeclarationLists((yyvsp[(4) - (10)].varDeclList).m_varDeclarations, (yyvsp[(10) - (10)].statementNode).m_varDeclarations),
                                                                                         mergeDeclarationLists((yyvsp[(4) - (10)].varDeclList).m_funcDeclarations, (yyvsp[(10) - (10)].statementNode).m_funcDeclarations),
                                                                                         (yyvsp[(4) - (10)].varDeclList).m_features | (yyvsp[(6) - (10)].expressionNode).m_features | (yyvsp[(8) - (10)].expressionNode).m_features | (yyvsp[(10) - (10)].statementNode).m_features,
                                                                                         (yyvsp[(4) - (10)].varDeclList).m_numConstants + (yyvsp[(6) - (10)].expressionNode).m_numConstants + (yyvsp[(8) - (10)].expressionNode).m_numConstants + (yyvsp[(10) - (10)].statementNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (10)]), (yylsp[(9) - (10)])); ;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 992 "..\\lib\\Parse\\ecmascript.y"
    {
                                            ForInNode* node = new ForInNode(GLOBAL_DATA, (yyvsp[(3) - (7)].expressionNode).m_node, (yyvsp[(5) - (7)].expressionNode).m_node, (yyvsp[(7) - (7)].statementNode).m_node);
                                            setExceptionLocation(node, (yylsp[(3) - (7)]).first_column, (yylsp[(3) - (7)]).last_column, (yylsp[(5) - (7)]).last_column);
                                            (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, (yyvsp[(7) - (7)].statementNode).m_varDeclarations, (yyvsp[(7) - (7)].statementNode).m_funcDeclarations,
                                                                                           (yyvsp[(3) - (7)].expressionNode).m_features | (yyvsp[(5) - (7)].expressionNode).m_features | (yyvsp[(7) - (7)].statementNode).m_features,
                                                                                           (yyvsp[(3) - (7)].expressionNode).m_numConstants + (yyvsp[(5) - (7)].expressionNode).m_numConstants + (yyvsp[(7) - (7)].statementNode).m_numConstants);
                                            setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (7)]), (yylsp[(6) - (7)]));
                                        ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1001 "..\\lib\\Parse\\ecmascript.y"
    { ForInNode *forIn = new ForInNode(GLOBAL_DATA, *(yyvsp[(4) - (8)].ident), 0, (yyvsp[(6) - (8)].expressionNode).m_node, (yyvsp[(8) - (8)].statementNode).m_node, (yylsp[(5) - (8)]).first_column, (yylsp[(5) - (8)]).first_column - (yylsp[(4) - (8)]).first_column, (yylsp[(6) - (8)]).last_column - (yylsp[(5) - (8)]).first_column);
                                          setExceptionLocation(forIn, (yylsp[(4) - (8)]).first_column, (yylsp[(5) - (8)]).first_column + 1, (yylsp[(6) - (8)]).last_column);
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyvsp[(8) - (8)].statementNode).m_varDeclarations, *(yyvsp[(4) - (8)].ident), DeclarationStacks::HasInitializer);
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(forIn, (yyvsp[(8) - (8)].statementNode).m_varDeclarations, (yyvsp[(8) - (8)].statementNode).m_funcDeclarations, ((*(yyvsp[(4) - (8)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | (yyvsp[(6) - (8)].expressionNode).m_features | (yyvsp[(8) - (8)].statementNode).m_features, (yyvsp[(6) - (8)].expressionNode).m_numConstants + (yyvsp[(8) - (8)].statementNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (8)]), (yylsp[(7) - (8)])); ;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1007 "..\\lib\\Parse\\ecmascript.y"
    { ForInNode *forIn = new ForInNode(GLOBAL_DATA, *(yyvsp[(4) - (9)].ident), (yyvsp[(5) - (9)].expressionNode).m_node, (yyvsp[(7) - (9)].expressionNode).m_node, (yyvsp[(9) - (9)].statementNode).m_node, (yylsp[(5) - (9)]).first_column, (yylsp[(5) - (9)]).first_column - (yylsp[(4) - (9)]).first_column, (yylsp[(5) - (9)]).last_column - (yylsp[(5) - (9)]).first_column);
                                          setExceptionLocation(forIn, (yylsp[(4) - (9)]).first_column, (yylsp[(6) - (9)]).first_column + 1, (yylsp[(7) - (9)]).last_column);
                                          appendToVarDeclarationList(GLOBAL_DATA, (yyvsp[(9) - (9)].statementNode).m_varDeclarations, *(yyvsp[(4) - (9)].ident), DeclarationStacks::HasInitializer);
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(forIn, (yyvsp[(9) - (9)].statementNode).m_varDeclarations, (yyvsp[(9) - (9)].statementNode).m_funcDeclarations,
                                                                                         ((*(yyvsp[(4) - (9)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | (yyvsp[(5) - (9)].expressionNode).m_features | (yyvsp[(7) - (9)].expressionNode).m_features | (yyvsp[(9) - (9)].statementNode).m_features,
                                                                                         (yyvsp[(5) - (9)].expressionNode).m_numConstants + (yyvsp[(7) - (9)].expressionNode).m_numConstants + (yyvsp[(9) - (9)].statementNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (9)]), (yylsp[(8) - (9)])); ;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1017 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(0, 0, 0); ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1022 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.expressionNode) = createNodeInfo<ExpressionNode*>(0, 0, 0); ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1027 "..\\lib\\Parse\\ecmascript.y"
    { ContinueNode* node = new ContinueNode(GLOBAL_DATA);
                                          setExceptionLocation(node, (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(1) - (2)]).last_column); 
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1031 "..\\lib\\Parse\\ecmascript.y"
    { ContinueNode* node = new ContinueNode(GLOBAL_DATA);
                                          setExceptionLocation(node, (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(1) - (2)]).last_column); 
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1035 "..\\lib\\Parse\\ecmascript.y"
    { ContinueNode* node = new ContinueNode(GLOBAL_DATA, *(yyvsp[(2) - (3)].ident));
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).last_column, (yylsp[(2) - (3)]).last_column); 
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1039 "..\\lib\\Parse\\ecmascript.y"
    { ContinueNode* node = new ContinueNode(GLOBAL_DATA, *(yyvsp[(2) - (3)].ident));
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).last_column, (yylsp[(2) - (3)]).last_column); 
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1046 "..\\lib\\Parse\\ecmascript.y"
    { BreakNode* node = new BreakNode(GLOBAL_DATA);
                                          setExceptionLocation(node, (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(1) - (2)]).last_column);
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0); setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1049 "..\\lib\\Parse\\ecmascript.y"
    { BreakNode* node = new BreakNode(GLOBAL_DATA);
                                          setExceptionLocation(node, (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(1) - (2)]).last_column);
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new BreakNode(GLOBAL_DATA), 0, 0, 0, 0); setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1052 "..\\lib\\Parse\\ecmascript.y"
    { BreakNode* node = new BreakNode(GLOBAL_DATA, *(yyvsp[(2) - (3)].ident));
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).last_column, (yylsp[(2) - (3)]).last_column);
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0); setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1055 "..\\lib\\Parse\\ecmascript.y"
    { BreakNode* node = new BreakNode(GLOBAL_DATA, *(yyvsp[(2) - (3)].ident));
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).last_column, (yylsp[(2) - (3)]).last_column);
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new BreakNode(GLOBAL_DATA, *(yyvsp[(2) - (3)].ident)), 0, 0, 0, 0); setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1061 "..\\lib\\Parse\\ecmascript.y"
    { ReturnNode* node = new ReturnNode(GLOBAL_DATA, 0); 
                                          setExceptionLocation(node, (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(1) - (2)]).last_column); 
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0); setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1064 "..\\lib\\Parse\\ecmascript.y"
    { ReturnNode* node = new ReturnNode(GLOBAL_DATA, 0); 
                                          setExceptionLocation(node, (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column, (yylsp[(1) - (2)]).last_column); 
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, 0, 0); setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1067 "..\\lib\\Parse\\ecmascript.y"
    { ReturnNode* node = new ReturnNode(GLOBAL_DATA, (yyvsp[(2) - (3)].expressionNode).m_node); 
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).last_column, (yylsp[(2) - (3)]).last_column);
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, (yyvsp[(2) - (3)].expressionNode).m_features, (yyvsp[(2) - (3)].expressionNode).m_numConstants); setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1070 "..\\lib\\Parse\\ecmascript.y"
    { ReturnNode* node = new ReturnNode(GLOBAL_DATA, (yyvsp[(2) - (3)].expressionNode).m_node); 
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).last_column, (yylsp[(2) - (3)]).last_column); 
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, (yyvsp[(2) - (3)].expressionNode).m_features, (yyvsp[(2) - (3)].expressionNode).m_numConstants); setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1076 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new WithNode(GLOBAL_DATA, (yyvsp[(3) - (5)].expressionNode).m_node, (yyvsp[(5) - (5)].statementNode).m_node, (yylsp[(3) - (5)]).last_column, (yylsp[(3) - (5)]).last_column - (yylsp[(3) - (5)]).first_column),
                                                                                         (yyvsp[(5) - (5)].statementNode).m_varDeclarations, (yyvsp[(5) - (5)].statementNode).m_funcDeclarations, (yyvsp[(3) - (5)].expressionNode).m_features | (yyvsp[(5) - (5)].statementNode).m_features | WithFeature, (yyvsp[(3) - (5)].expressionNode).m_numConstants + (yyvsp[(5) - (5)].statementNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1082 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new SwitchNode(GLOBAL_DATA, (yyvsp[(3) - (5)].expressionNode).m_node, (yyvsp[(5) - (5)].caseBlockNode).m_node), (yyvsp[(5) - (5)].caseBlockNode).m_varDeclarations, (yyvsp[(5) - (5)].caseBlockNode).m_funcDeclarations,
                                                                                         (yyvsp[(3) - (5)].expressionNode).m_features | (yyvsp[(5) - (5)].caseBlockNode).m_features, (yyvsp[(3) - (5)].expressionNode).m_numConstants + (yyvsp[(5) - (5)].caseBlockNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1088 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.caseBlockNode) = createNodeDeclarationInfo<CaseBlockNode*>(new CaseBlockNode(GLOBAL_DATA, (yyvsp[(2) - (3)].clauseList).m_node.head, 0, 0), (yyvsp[(2) - (3)].clauseList).m_varDeclarations, (yyvsp[(2) - (3)].clauseList).m_funcDeclarations, (yyvsp[(2) - (3)].clauseList).m_features, (yyvsp[(2) - (3)].clauseList).m_numConstants); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1090 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.caseBlockNode) = createNodeDeclarationInfo<CaseBlockNode*>(new CaseBlockNode(GLOBAL_DATA, (yyvsp[(2) - (5)].clauseList).m_node.head, (yyvsp[(3) - (5)].caseClauseNode).m_node, (yyvsp[(4) - (5)].clauseList).m_node.head),
                                                                                         mergeDeclarationLists(mergeDeclarationLists((yyvsp[(2) - (5)].clauseList).m_varDeclarations, (yyvsp[(3) - (5)].caseClauseNode).m_varDeclarations), (yyvsp[(4) - (5)].clauseList).m_varDeclarations),
                                                                                         mergeDeclarationLists(mergeDeclarationLists((yyvsp[(2) - (5)].clauseList).m_funcDeclarations, (yyvsp[(3) - (5)].caseClauseNode).m_funcDeclarations), (yyvsp[(4) - (5)].clauseList).m_funcDeclarations),
                                                                                         (yyvsp[(2) - (5)].clauseList).m_features | (yyvsp[(3) - (5)].caseClauseNode).m_features | (yyvsp[(4) - (5)].clauseList).m_features,
                                                                                         (yyvsp[(2) - (5)].clauseList).m_numConstants + (yyvsp[(3) - (5)].caseClauseNode).m_numConstants + (yyvsp[(4) - (5)].clauseList).m_numConstants); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1098 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.clauseList).m_node.head = 0; (yyval.clauseList).m_node.tail = 0; (yyval.clauseList).m_varDeclarations = 0; (yyval.clauseList).m_funcDeclarations = 0; (yyval.clauseList).m_features = 0; (yyval.clauseList).m_numConstants = 0; ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1103 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.clauseList).m_node.head = new ClauseListNode(GLOBAL_DATA, (yyvsp[(1) - (1)].caseClauseNode).m_node);
                                          (yyval.clauseList).m_node.tail = (yyval.clauseList).m_node.head;
                                          (yyval.clauseList).m_varDeclarations = (yyvsp[(1) - (1)].caseClauseNode).m_varDeclarations;
                                          (yyval.clauseList).m_funcDeclarations = (yyvsp[(1) - (1)].caseClauseNode).m_funcDeclarations; 
                                          (yyval.clauseList).m_features = (yyvsp[(1) - (1)].caseClauseNode).m_features;
                                          (yyval.clauseList).m_numConstants = (yyvsp[(1) - (1)].caseClauseNode).m_numConstants; ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1109 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.clauseList).m_node.head = (yyvsp[(1) - (2)].clauseList).m_node.head;
                                          (yyval.clauseList).m_node.tail = new ClauseListNode(GLOBAL_DATA, (yyvsp[(2) - (2)].caseClauseNode).m_node, (yyvsp[(1) - (2)].clauseList).m_node.tail);
                                          (yyval.clauseList).m_varDeclarations = mergeDeclarationLists((yyvsp[(1) - (2)].clauseList).m_varDeclarations, (yyvsp[(2) - (2)].caseClauseNode).m_varDeclarations);
                                          (yyval.clauseList).m_funcDeclarations = mergeDeclarationLists((yyvsp[(1) - (2)].clauseList).m_funcDeclarations, (yyvsp[(2) - (2)].caseClauseNode).m_funcDeclarations);
                                          (yyval.clauseList).m_features = (yyvsp[(1) - (2)].clauseList).m_features | (yyvsp[(2) - (2)].caseClauseNode).m_features;
                                          (yyval.clauseList).m_numConstants = (yyvsp[(1) - (2)].clauseList).m_numConstants + (yyvsp[(2) - (2)].caseClauseNode).m_numConstants;
                                        ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1119 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.caseClauseNode) = createNodeDeclarationInfo<CaseClauseNode*>(new CaseClauseNode(GLOBAL_DATA, (yyvsp[(2) - (3)].expressionNode).m_node), 0, 0, (yyvsp[(2) - (3)].expressionNode).m_features, (yyvsp[(2) - (3)].expressionNode).m_numConstants); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1120 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.caseClauseNode) = createNodeDeclarationInfo<CaseClauseNode*>(new CaseClauseNode(GLOBAL_DATA, (yyvsp[(2) - (4)].expressionNode).m_node, (yyvsp[(4) - (4)].sourceElements).m_node), (yyvsp[(4) - (4)].sourceElements).m_varDeclarations, (yyvsp[(4) - (4)].sourceElements).m_funcDeclarations, (yyvsp[(2) - (4)].expressionNode).m_features | (yyvsp[(4) - (4)].sourceElements).m_features, (yyvsp[(2) - (4)].expressionNode).m_numConstants + (yyvsp[(4) - (4)].sourceElements).m_numConstants); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1124 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.caseClauseNode) = createNodeDeclarationInfo<CaseClauseNode*>(new CaseClauseNode(GLOBAL_DATA, 0), 0, 0, 0, 0); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1125 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.caseClauseNode) = createNodeDeclarationInfo<CaseClauseNode*>(new CaseClauseNode(GLOBAL_DATA, 0, (yyvsp[(3) - (3)].sourceElements).m_node), (yyvsp[(3) - (3)].sourceElements).m_varDeclarations, (yyvsp[(3) - (3)].sourceElements).m_funcDeclarations, (yyvsp[(3) - (3)].sourceElements).m_features, (yyvsp[(3) - (3)].sourceElements).m_numConstants); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1129 "..\\lib\\Parse\\ecmascript.y"
    { LabelNode* node = new LabelNode(GLOBAL_DATA, *(yyvsp[(1) - (3)].ident), (yyvsp[(3) - (3)].statementNode).m_node);
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).last_column, (yylsp[(2) - (3)]).last_column);
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, (yyvsp[(3) - (3)].statementNode).m_varDeclarations, (yyvsp[(3) - (3)].statementNode).m_funcDeclarations, (yyvsp[(3) - (3)].statementNode).m_features, (yyvsp[(3) - (3)].statementNode).m_numConstants); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1135 "..\\lib\\Parse\\ecmascript.y"
    { ThrowNode* node = new ThrowNode(GLOBAL_DATA, (yyvsp[(2) - (3)].expressionNode).m_node);
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).last_column, (yylsp[(2) - (3)]).last_column);
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, (yyvsp[(2) - (3)].expressionNode).m_features, (yyvsp[(2) - (3)].expressionNode).m_numConstants); setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(2) - (3)]));
                                        ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1139 "..\\lib\\Parse\\ecmascript.y"
    { ThrowNode* node = new ThrowNode(GLOBAL_DATA, (yyvsp[(2) - (3)].expressionNode).m_node);
                                          setExceptionLocation(node, (yylsp[(1) - (3)]).first_column, (yylsp[(2) - (3)]).last_column, (yylsp[(2) - (3)]).last_column);
                                          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(node, 0, 0, (yyvsp[(2) - (3)].expressionNode).m_features, (yyvsp[(2) - (3)].expressionNode).m_numConstants); setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; 
                                        ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1146 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new TryNode(GLOBAL_DATA, (yyvsp[(2) - (4)].statementNode).m_node, GLOBAL_DATA->propertyNames->nullIdentifier, false, 0, (yyvsp[(4) - (4)].statementNode).m_node),
                                                                                         mergeDeclarationLists((yyvsp[(2) - (4)].statementNode).m_varDeclarations, (yyvsp[(4) - (4)].statementNode).m_varDeclarations),
                                                                                         mergeDeclarationLists((yyvsp[(2) - (4)].statementNode).m_funcDeclarations, (yyvsp[(4) - (4)].statementNode).m_funcDeclarations),
                                                                                         (yyvsp[(2) - (4)].statementNode).m_features | (yyvsp[(4) - (4)].statementNode).m_features,
                                                                                         (yyvsp[(2) - (4)].statementNode).m_numConstants + (yyvsp[(4) - (4)].statementNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (4)]), (yylsp[(2) - (4)])); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1152 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new TryNode(GLOBAL_DATA, (yyvsp[(2) - (7)].statementNode).m_node, *(yyvsp[(5) - (7)].ident), ((yyvsp[(7) - (7)].statementNode).m_features & EvalFeature) != 0, (yyvsp[(7) - (7)].statementNode).m_node, 0),
                                                                                         mergeDeclarationLists((yyvsp[(2) - (7)].statementNode).m_varDeclarations, (yyvsp[(7) - (7)].statementNode).m_varDeclarations),
                                                                                         mergeDeclarationLists((yyvsp[(2) - (7)].statementNode).m_funcDeclarations, (yyvsp[(7) - (7)].statementNode).m_funcDeclarations),
                                                                                         (yyvsp[(2) - (7)].statementNode).m_features | (yyvsp[(7) - (7)].statementNode).m_features | CatchFeature,
                                                                                         (yyvsp[(2) - (7)].statementNode).m_numConstants + (yyvsp[(7) - (7)].statementNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (7)]), (yylsp[(2) - (7)])); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1159 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new TryNode(GLOBAL_DATA, (yyvsp[(2) - (9)].statementNode).m_node, *(yyvsp[(5) - (9)].ident), ((yyvsp[(7) - (9)].statementNode).m_features & EvalFeature) != 0, (yyvsp[(7) - (9)].statementNode).m_node, (yyvsp[(9) - (9)].statementNode).m_node),
                                                                                         mergeDeclarationLists(mergeDeclarationLists((yyvsp[(2) - (9)].statementNode).m_varDeclarations, (yyvsp[(7) - (9)].statementNode).m_varDeclarations), (yyvsp[(9) - (9)].statementNode).m_varDeclarations),
                                                                                         mergeDeclarationLists(mergeDeclarationLists((yyvsp[(2) - (9)].statementNode).m_funcDeclarations, (yyvsp[(7) - (9)].statementNode).m_funcDeclarations), (yyvsp[(9) - (9)].statementNode).m_funcDeclarations),
                                                                                         (yyvsp[(2) - (9)].statementNode).m_features | (yyvsp[(7) - (9)].statementNode).m_features | (yyvsp[(9) - (9)].statementNode).m_features | CatchFeature,
                                                                                         (yyvsp[(2) - (9)].statementNode).m_numConstants + (yyvsp[(7) - (9)].statementNode).m_numConstants + (yyvsp[(9) - (9)].statementNode).m_numConstants);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (9)]), (yylsp[(2) - (9)])); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1168 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new DebuggerStatementNode(GLOBAL_DATA), 0, 0, 0, 0);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1170 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new DebuggerStatementNode(GLOBAL_DATA), 0, 0, 0, 0);
                                          setStatementLocation((yyval.statementNode).m_node, (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1175 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new FuncDeclNode(GLOBAL_DATA, *(yyvsp[(2) - (7)].ident), (yyvsp[(6) - (7)].functionBodyNode)), 0, new ParserArenaData<DeclarationStacks::FunctionStack>, ((*(yyvsp[(2) - (7)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | ClosureFeature, 0); setStatementLocation((yyvsp[(6) - (7)].functionBodyNode), (yylsp[(5) - (7)]), (yylsp[(7) - (7)])); (yyval.statementNode).m_funcDeclarations->data.push_back(static_cast<FuncDeclNode*>((yyval.statementNode).m_node)->body()); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1177 "..\\lib\\Parse\\ecmascript.y"
    {
          (yyval.statementNode) = createNodeDeclarationInfo<StatementNode*>(new FuncDeclNode(GLOBAL_DATA, *(yyvsp[(2) - (8)].ident), (yyvsp[(7) - (8)].functionBodyNode), (yyvsp[(4) - (8)].parameterList).m_node.head), 0, new ParserArenaData<DeclarationStacks::FunctionStack>, ((*(yyvsp[(2) - (8)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0) | (yyvsp[(4) - (8)].parameterList).m_features | ClosureFeature, 0);
          if ((yyvsp[(4) - (8)].parameterList).m_features & ArgumentsFeature)
              (yyvsp[(7) - (8)].functionBodyNode)->setUsesArguments();
          setStatementLocation((yyvsp[(7) - (8)].functionBodyNode), (yylsp[(6) - (8)]), (yylsp[(8) - (8)]));
          (yyval.statementNode).m_funcDeclarations->data.push_back(static_cast<FuncDeclNode*>((yyval.statementNode).m_node)->body());
      ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1187 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.funcExprNode) = createNodeInfo(new FuncExprNode(GLOBAL_DATA, GLOBAL_DATA->propertyNames->nullIdentifier, (yyvsp[(5) - (6)].functionBodyNode)), ClosureFeature, 0); setStatementLocation((yyvsp[(5) - (6)].functionBodyNode), (yylsp[(4) - (6)]), (yylsp[(6) - (6)])); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1189 "..\\lib\\Parse\\ecmascript.y"
    {
          (yyval.funcExprNode) = createNodeInfo(new FuncExprNode(GLOBAL_DATA, GLOBAL_DATA->propertyNames->nullIdentifier, (yyvsp[(6) - (7)].functionBodyNode), (yyvsp[(3) - (7)].parameterList).m_node.head), (yyvsp[(3) - (7)].parameterList).m_features | ClosureFeature, 0);
          if ((yyvsp[(3) - (7)].parameterList).m_features & ArgumentsFeature)
              (yyvsp[(6) - (7)].functionBodyNode)->setUsesArguments();
          setStatementLocation((yyvsp[(6) - (7)].functionBodyNode), (yylsp[(5) - (7)]), (yylsp[(7) - (7)]));
      ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1195 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.funcExprNode) = createNodeInfo(new FuncExprNode(GLOBAL_DATA, *(yyvsp[(2) - (7)].ident), (yyvsp[(6) - (7)].functionBodyNode)), ClosureFeature, 0); setStatementLocation((yyvsp[(6) - (7)].functionBodyNode), (yylsp[(5) - (7)]), (yylsp[(7) - (7)])); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1197 "..\\lib\\Parse\\ecmascript.y"
    {
          (yyval.funcExprNode) = createNodeInfo(new FuncExprNode(GLOBAL_DATA, *(yyvsp[(2) - (8)].ident), (yyvsp[(7) - (8)].functionBodyNode), (yyvsp[(4) - (8)].parameterList).m_node.head), (yyvsp[(4) - (8)].parameterList).m_features | ClosureFeature, 0); 
          if ((yyvsp[(4) - (8)].parameterList).m_features & ArgumentsFeature)
              (yyvsp[(7) - (8)].functionBodyNode)->setUsesArguments();
          setStatementLocation((yyvsp[(7) - (8)].functionBodyNode), (yylsp[(6) - (8)]), (yylsp[(8) - (8)]));
      ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1206 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.parameterList).m_node.head = new ParameterNode(GLOBAL_DATA, *(yyvsp[(1) - (1)].ident));
                                          (yyval.parameterList).m_features = (*(yyvsp[(1) - (1)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0;
                                          (yyval.parameterList).m_node.tail = (yyval.parameterList).m_node.head; ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1209 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.parameterList).m_node.head = (yyvsp[(1) - (3)].parameterList).m_node.head;
                                          (yyval.parameterList).m_features = (yyvsp[(1) - (3)].parameterList).m_features | ((*(yyvsp[(3) - (3)].ident) == GLOBAL_DATA->propertyNames->arguments) ? ArgumentsFeature : 0);
                                          (yyval.parameterList).m_node.tail = new ParameterNode(GLOBAL_DATA, *(yyvsp[(3) - (3)].ident), (yyvsp[(1) - (3)].parameterList).m_node.tail);  ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1215 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.functionBodyNode) = FunctionBodyNode::create(GLOBAL_DATA); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1216 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.functionBodyNode) = FunctionBodyNode::create(GLOBAL_DATA, (yyvsp[(1) - (1)].sourceElements).m_node, (yyvsp[(1) - (1)].sourceElements).m_varDeclarations ? &(yyvsp[(1) - (1)].sourceElements).m_varDeclarations->data : 0, 
                                                       (yyvsp[(1) - (1)].sourceElements).m_funcDeclarations ? &(yyvsp[(1) - (1)].sourceElements).m_funcDeclarations->data : 0,
                                                       ((yyvsp[(1) - (1)].sourceElements).m_features & EvalFeature) != 0, ((yyvsp[(1) - (1)].sourceElements).m_features & ClosureFeature) != 0);
                                        ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1223 "..\\lib\\Parse\\ecmascript.y"
    { ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1224 "..\\lib\\Parse\\ecmascript.y"
    { programBlock = (yyvsp[(1) - (1)].sourceElements).m_node; ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1228 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.sourceElements).m_node = new SourceElements(GLOBAL_DATA);
                                          (yyval.sourceElements).m_node->append((yyvsp[(1) - (1)].statementNode).m_node);
                                          (yyval.sourceElements).m_varDeclarations = (yyvsp[(1) - (1)].statementNode).m_varDeclarations;
                                          (yyval.sourceElements).m_funcDeclarations = (yyvsp[(1) - (1)].statementNode).m_funcDeclarations;
                                          (yyval.sourceElements).m_features = (yyvsp[(1) - (1)].statementNode).m_features;
                                          (yyval.sourceElements).m_numConstants = (yyvsp[(1) - (1)].statementNode).m_numConstants;
                                        ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1235 "..\\lib\\Parse\\ecmascript.y"
    { (yyval.sourceElements).m_node->append((yyvsp[(2) - (2)].statementNode).m_node);
                                          (yyval.sourceElements).m_varDeclarations = mergeDeclarationLists((yyvsp[(1) - (2)].sourceElements).m_varDeclarations, (yyvsp[(2) - (2)].statementNode).m_varDeclarations);
                                          (yyval.sourceElements).m_funcDeclarations = mergeDeclarationLists((yyvsp[(1) - (2)].sourceElements).m_funcDeclarations, (yyvsp[(2) - (2)].statementNode).m_funcDeclarations);
                                          (yyval.sourceElements).m_features = (yyvsp[(1) - (2)].sourceElements).m_features | (yyvsp[(2) - (2)].statementNode).m_features;
                                          (yyval.sourceElements).m_numConstants = (yyvsp[(1) - (2)].sourceElements).m_numConstants + (yyvsp[(2) - (2)].statementNode).m_numConstants;
                                        ;}
    break;



/* Line 1455 of yacc.c  */
#line 4194 "..\\lib\\Parse\\Parser.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 1243 "..\\lib\\Parse\\ecmascript.y"


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


%{
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

#include <string>
#include "NodeInfo.h"
#define PARSE_TOKEN  do { std::string num(yytext, yyleng); yylval.doubleValue = atof(num.c_str()); } while(0)
#define SAVE_TOKEN yylval.ident = new Identifier(yytext, yyleng)
#define SAVE_STRING yylval.ident = new Identifier(yytext+1, yyleng-2)
#define SAVE_REGEXP yylval.ident = new Identifier(yytext+1, yyleng-1)
#define TOKEN(t) (t)
#define YY_DECL int yylex(void *globalPtr)
%}
%option noyywrap

digit			[0-9]
letter			[a-zA-Z]
number			[0-9]+|[0-9]+\.[0-9]*
identifier		[a-zA-Z$_][a-zA-Z0-9$_]*
string			\"[^\n\"]+\"|\'[^\n\']+\'
line_comment	"//"[^\n]*
block_comment	"/*"([^\*]|(\*)*[^\*/])*(\*)*"*/"
regexp          "/"[^ \t\r\n\*/][^ \t\r\n/]+"/"[gimy]*
%%

[ \t\r\n]		;
{line_comment}	;
{block_comment}	;
"null"			return TOKEN(NULLTOKEN);
"true"			return TOKEN(TRUETOKEN);
"false"			return TOKEN(FALSETOKEN);
"break"			return TOKEN(BREAK);
"case"			return TOKEN(CASE);
"default"		return TOKEN(DEFAULT);
"for"			return TOKEN(FOR);
"new"			return TOKEN(NEW);
"var"			return TOKEN(VAR);
"const"			return TOKEN(CONSTTOKEN);
"continue"		return TOKEN(CONTINUE);
"function"		return TOKEN(FUNCTION);
"return"		return TOKEN(RETURN);
"void"			return TOKEN(VOIDTOKEN);
"delete"		return TOKEN(DELETETOKEN);
"if"			return TOKEN(IF);
"else"			return TOKEN(ELSE);
"this"			return TOKEN(THISTOKEN);
"do"			return TOKEN(DO);
"while"			return TOKEN(WHILE);
"in"			return TOKEN(INTOKEN);
"instanceof"	return TOKEN(INSTANCEOF);
"typeof"		return TOKEN(TYPEOF);
"switch"		return TOKEN(SWITCH);
"with"			return TOKEN(WITH);
"reserved"		return TOKEN(RESERVED);
"throw"			return TOKEN(THROW);
"try"			return TOKEN(TRY);
"catch"			return TOKEN(CATCH);
"finally"		return TOKEN(FINALLY);
"debugger"		return TOKEN(DEBUGGER);
"=="			return TOKEN(EQEQ);
"!="			return TOKEN(NE);
"==="			return TOKEN(STREQ);
"!=="			return TOKEN(STRNEQ);
"<="			return TOKEN(LE);
">="			return TOKEN(GE);
"||"			return TOKEN(OR);
"&&"			return TOKEN(AND);
"++"			return TOKEN(PLUSPLUS);
"--"			return TOKEN(MINUSMINUS);
"<<"			return TOKEN(LSHIFT);
">>"			return TOKEN(RSHIFT);
">>>"			return TOKEN(URSHIFT);
"+="			return TOKEN(PLUSEQUAL);
"-="			return TOKEN(MINUSEQUAL);
"*="			return TOKEN(MULTEQUAL);
"/="			return TOKEN(DIVEQUAL);
"<<="			return TOKEN(LSHIFTEQUAL);
">>="			return TOKEN(RSHIFTEQUAL);
">>>="			return TOKEN(URSHIFTEQUAL);
"&="			return TOKEN(ANDEQUAL);
"%="			return TOKEN(MODEQUAL);
"^="			return TOKEN(XOREQUAL);
"|="			return TOKEN(OREQUAL);
"{"				return TOKEN(OPENBRACE);
"}"				return TOKEN(CLOSEBRACE);
{number}		PARSE_TOKEN; return NUMBER;
{identifier}	SAVE_TOKEN; return IDENT;
{string}		SAVE_STRING; return STRING;
{regexp}        SAVE_REGEXP; return REGEXP;
"<"				return TOKEN(LT);
">"				return TOKEN(GT);
"="				return TOKEN(EQUAL);
"!"				return TOKEN(NOT);
"|"				return TOKEN(BITOR);
"&"				return TOKEN(BITAND);
"^"				return TOKEN(BITXOR);
"~"				return TOKEN(BITINV);
"("				return TOKEN(LPAREN);
")"				return TOKEN(RPAREN);
"["				return TOKEN(LBRACKET);
"]"				return TOKEN(RBRACKET);
"?"				return TOKEN(QUESTIONMARK);
"."				return TOKEN(DOT);
":"				return TOKEN(COLON);
";"				return TOKEN(SEMICOLON);
","				return TOKEN(COMMA);
"+"				return TOKEN(PLUS);
"-"				return TOKEN(MINUS);
"*"				return TOKEN(MUL);
"/"				return TOKEN(DIV);
"%"				return TOKEN(MOD);
.				printf("Unknown token(%c)!\n", *yytext); yyterminate();

%%
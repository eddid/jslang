@echo off
flex -o..\lib\Lex\Lexer.cpp ..\lib\Lex\ecmascript.l
bison -d -o ..\lib\Parse\Parser.cpp ..\lib\Parse\ecmascript.y
@mv ..\lib\Parse\Parser.hpp ..\include\jslang\Parse\Parser.hpp
@pause
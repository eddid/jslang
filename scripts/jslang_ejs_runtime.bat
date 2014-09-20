C:\MyProject\llvm-3.4.2\bin\Debug\clang.exe -S -emit-llvm ..\ejs_runtime\allinone_for_load.c -imacros ..\ejs_runtime\macros.h -o ..\build\allinone.ll
REM C:\MyProject\llvm-3.4.2\bin\Debug\clang.exe ..\ejs_runtime\allinone_for_load.c -imacros ..\ejs_runtime\macros.h
REM C:\MyProject\llvm-3.4.2\bin\Debug\llc.exe -march=cpp ..\build\allinone.ll -o ..\build\allinone_llvm.cpp
@pause
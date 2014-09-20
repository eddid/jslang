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
#include "NodeConstructors.h"
#include <iostream>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#if JSLANG_VER == 2
#include "JSTypeTree.h"
#endif

#include <Windows.h>
#pragma comment(lib, "Winmm.lib")

using namespace JSC;
using namespace std;

extern int yyparse(void *YYPARSE_PARAM);
extern SourceElements* programBlock;
extern FILE* yyin;
extern int yydebug;

void printfunctions(Module *module)
{
  Module::const_iterator fI  = module->begin();
  Module::const_iterator fIE = module->end();

  for (; fI != fIE; ++fI) {
    const Function &func = *fI;
    std::string name = func.getName();
    std::cout << name.c_str() << std::endl;
  }

  Module::const_global_iterator vI  = module->global_begin();
  Module::const_global_iterator vIE = module->global_end();

  for (; vI != vIE; ++vI) {
    const GlobalVariable &var = *vI;
    std::string name = var.getName();
    std::cout << name.c_str() << std::endl;
  }
}

int main(int argc, char **argv)
{
    // see http://comments.gmane.org/gmane.comp.compilers.llvm.devel/33877
	InitializeNativeTarget();
    SMDiagnostic error;
    Module *module = ParseIRFile("allinone.ll", error, getGlobalContext());
    //printfunctions(module);
    //delete module;module = NULL;
	CodeGenContext context(module);
	context.init();
#if JSLANG_VER == 2
	JSTypeTree::Init(context);
#endif
	
	char *inputFile = "../test/example.js";
    if (argc > 1)
	{
		inputFile = argv[1];
	}
	char outputFile[MAX_PATH];
    if (argc > 2)
	{
	    strncpy(outputFile, argv[2], sizeof(outputFile));
	}
	else
	{
	    snprintf(outputFile, sizeof(outputFile), "%s.obj", inputFile);
	}
	yydebug = 0;
	yyin = fopen(inputFile, "rb+");
	yyparse((void *)&context);
	std::cout << programBlock << endl;
	if (NULL != programBlock)
	{
    	context.generateCode(*programBlock);
    	DWORD startTime = timeGetTime();
    	//context.runCode();
    	context.compileCode(outputFile);
    	DWORD endTime = timeGetTime();
    	double deltaTime = (double)(endTime - startTime) / 1000;
    	std::cout << "time: " << deltaTime << endl;
	}
	
	if (NULL != yyin)
	{
		fclose(yyin);
	}
	return 0;
}

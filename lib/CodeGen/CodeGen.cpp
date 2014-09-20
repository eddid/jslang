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
#include "llvm/ADT/Triple.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Target/TargetMachine.h"

using namespace JSC;
using namespace std;

void declareVariables(CodeGenContext& context);
void declareFunctions(CodeGenContext& context);
void initExternals(CodeGenContext &context, ExecutionEngine *ee);
void registVariables(CodeGenContext& context);
void CodeGenContext::init()
{
    declareVariables(*this);
    declareFunctions(*this);
    registVariables(*this);
}

/* Compile the AST into a module */
void CodeGenContext::generateCode(SourceElements& root)
{
	std::cout << "Generating code...\n";
	
	/* Create the top level interpreter function to call as entry */
    llvm::IntegerType *int32Type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
    llvm::PointerType *int8PtrType = llvm::Type::getInt8PtrTy(llvm::getGlobalContext());
    llvm::PointerType *int8PtrPtrType = llvm::PointerType::get(int8PtrType, 0);
	vector<Type*> argTypes;
    argTypes.push_back(int32Type);
    argTypes.push_back(int8PtrPtrType);
	FunctionType *ftype = FunctionType::get(int32Type, makeArrayRef(argTypes), false);
	mainFunc = Function::Create(ftype, GlobalValue::ExternalLinkage, "main", module);
    mainFunc->setCallingConv(llvm::CallingConv::C);
	BasicBlock *entryBB = BasicBlock::Create(getGlobalContext(), "entry", mainFunc, 0);
	
	/* Push a new variable/block context */
	pushBlock(entryBB);
	//call printtimestamp();
	llvm::Function *function_printtimestamp = getFunction("jsextern_print_tick");
	if (NULL == function_printtimestamp)
	{
	    std::cout << "Can't find jsextern_print_tick()" << std::endl;
	    abort();
	    return ;
	}
	llvm::CallInst::Create(function_printtimestamp, "", currentBlock());

	//call AllInOneInit();
	llvm::Function *function_allinone = getFunction("AllInOneInit");
	if (NULL == function_allinone)
	{
	    std::cout << "Can't find AllInOneInit()" << std::endl;
	    abort();
	    return ;
	}
	llvm::CallInst::Create(function_allinone, "", currentBlock());

	//CodeGenJSValue32ConstructForNULL(*this, "_ejs_null");
	//CodeGenJSValue32ConstructForUndefined(*this, "_ejs_undefined");
	//CodeGenJSValue32ConstructForBool(*this, true, "jsBool");
	//CodeGenJSValue32ConstructForInt32(*this, 6, "jsInt");
	//RegisterID *jsDoubleRef = CodeGenJSValue32ConstructForDouble(*this, 1.234, "jsDouble");
	//RegisterID *jsDouble = new LoadInst(jsDoubleRef, "", false, entryBB);
	//CodeGenJSValue32FunctionToBool(*this, jsDouble);
	root.emitBytecode(*this, NULL); /* emit bytecode for the toplevel block */
	llvm::CallInst::Create(function_printtimestamp, "", currentBlock());
	ReturnInst::Create(getGlobalContext(), llvm::ConstantInt::get(int32Type, 0), currentBlock());
	popBlock(entryBB);
	
	/* Print the bytecode in a human-readable format 
	   to see if our program compiled properly
	 */
	std::cout << "Code is generated.\n";
	PassManager pm;
	pm.add(createPrintModulePass(&outs()));
	pm.run(*module);
}

/* Executes the AST by running the main function */
/* like tools/lli.cpp */
GenericValue CodeGenContext::runCode() {
	std::cout << "Running code...\n";
	ExecutionEngine *ee = EngineBuilder(module).create();

    initExternals(*this, ee);

    std::vector<GenericValue> Args;
    GenericValue GVArgc;
    GVArgc.IntVal = APInt(32, 0);
    Args.push_back(GVArgc);
    Args.push_back(PTOGV(NULL));
	GenericValue v = ee->runFunction(mainFunc, Args);
	std::cout << "Code was run.\n";
	return v;
}

static int compileModule(Module *mod, LLVMContext &Context, char *inputFile) {
  // Load the module to be compiled...
  SMDiagnostic Err;
  Triple TheTriple;

    if (mod == 0) {
      std::cout << "module=NULL" << std::endl;
      return 1;
    }

    // If we are supposed to override the target triple, do so now.
    TheTriple = Triple(mod->getTargetTriple());

  if (TheTriple.getTriple().empty())
    TheTriple.setTriple(sys::getDefaultTargetTriple());

  // Get the target specific parser.
  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget(MArch, TheTriple,
                                                         Error);
  if (!TheTarget) {
    std::cout << Error;
    return 1;
  }

  // Package up features to be passed to target/subtarget
  std::string FeaturesStr;
  if (MAttrs.size()) {
    SubtargetFeatures Features;
    for (unsigned i = 0; i != MAttrs.size(); ++i)
      Features.AddFeature(MAttrs[i]);
    FeaturesStr = Features.getString();
  }

  CodeGenOpt::Level OLvl = CodeGenOpt::Default;

  TargetOptions Options;
  Options.LessPreciseFPMADOption = EnableFPMAD;
  Options.NoFramePointerElim = DisableFPElim;
  Options.AllowFPOpFusion = FuseFPOps;
  Options.UnsafeFPMath = EnableUnsafeFPMath;
  Options.NoInfsFPMath = EnableNoInfsFPMath;
  Options.NoNaNsFPMath = EnableNoNaNsFPMath;
  Options.HonorSignDependentRoundingFPMathOption =
      EnableHonorSignDependentRoundingFPMath;
  Options.UseSoftFloat = GenerateSoftFloatCalls;
  if (FloatABIForCalls != FloatABI::Default)
    Options.FloatABIType = FloatABIForCalls;
  Options.NoZerosInBSS = DontPlaceZerosInBSS;
  Options.GuaranteedTailCallOpt = EnableGuaranteedTailCallOpt;
  Options.DisableTailCalls = DisableTailCalls;
  Options.StackAlignmentOverride = OverrideStackAlignment;
  Options.TrapFuncName = TrapFuncName;
  Options.PositionIndependentExecutable = EnablePIE;
  Options.EnableSegmentedStacks = SegmentedStacks;
  Options.UseInitArray = UseInitArray;

  OwningPtr<TargetMachine>
    target(TheTarget->createTargetMachine(TheTriple.getTriple(),
                                          MCPU, FeaturesStr, Options,
                                          RelocModel, CMModel, OLvl));
  assert(target.get() && "Could not allocate target machine!");
  assert(mod && "Should have exited after outputting help!");
  TargetMachine &Target = *target.get();

  if (DisableDotLoc)
    Target.setMCUseLoc(false);

  if (DisableCFI)
    Target.setMCUseCFI(false);

  if (EnableDwarfDirectory)
    Target.setMCUseDwarfDirectory(true);

  if (GenerateSoftFloatCalls)
    FloatABIForCalls = FloatABI::Soft;

  // Disable .loc support for older OS X versions.
  if (TheTriple.isMacOSX() &&
      TheTriple.isMacOSXVersionLT(10, 6))
    Target.setMCUseLoc(false);

  // Figure out where we are going to send the output.
  std::string error;
#if 0
  std::string output_filename = inputFile;
  if (TheTriple.getOS() == Triple::Win32)
      output_filename += ".obj";
  else
      output_filename += ".o";
#endif

  OwningPtr<tool_output_file> Out(new tool_output_file(inputFile, error, sys::fs::F_Binary));
  if (!Out) return 1;

  // Build up all of the passes that we want to do to the module.
  PassManager PM;

  // Add an appropriate TargetLibraryInfo pass for the module's triple.
  TargetLibraryInfo *TLI = new TargetLibraryInfo(TheTriple);
  PM.add(TLI);

  // Add intenal analysis passes from the target machine.
  Target.addAnalysisPasses(PM);

  // Add the target data from the target machine, if it exists, or the module.
  if (const DataLayout *TD = Target.getDataLayout())
    PM.add(new DataLayout(*TD));
  else
    PM.add(new DataLayout(mod));

  // Override default to generate verbose assembly.
  Target.setAsmVerbosityDefault(true);

  if (RelaxAll) {
    if (FileType != TargetMachine::CGFT_ObjectFile)
      std::cout << "warning: ignoring -mc-relax-all because filetype != obj";
    else
      Target.setMCRelaxAll(true);
  }

  {
    formatted_raw_ostream FOS(Out->os());

    AnalysisID StartAfterID = 0;
    AnalysisID StopAfterID = 0;
    const PassRegistry *PR = PassRegistry::getPassRegistry();
    if (!StartAfter.empty()) {
      const PassInfo *PI = PR->getPassInfo(StartAfter);
      if (!PI) {
        std::cout << "start-after pass is not registered.\n";
        return 1;
      }
      StartAfterID = PI->getTypeInfo();
    }
    if (!StopAfter.empty()) {
      const PassInfo *PI = PR->getPassInfo(StopAfter);
      if (!PI) {
        std::cout << "stop-after pass is not registered.\n";
        return 1;
      }
      StopAfterID = PI->getTypeInfo();
    }

    // Ask the target to add backend passes as necessary.
    if (Target.addPassesToEmitFile(PM, FOS, FileType, true, StartAfterID, StopAfterID)) {
      std::cout << "target does not support generation of this file type!\n";
      return 1;
    }

    // Before executing passes, print the final values of the LLVM options.
    cl::PrintOptionValues();

    PM.run(*mod);
  }

  // Declare success.
  Out->keep();

  return 0;
}

/* Compiles the AST by compiling the whole module */
/* like tools/llc.cpp */
void CodeGenContext::compileCode(char *inputFile) {
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(0, 0);

  // Enable debug stream buffering.
  EnableDebugBuffering = true;
  FileType = TargetMachine::CGFT_ObjectFile;
  MArch = "x86";

  LLVMContext &Context = getGlobalContext();
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.

  // Initialize targets first, so that --version shows registered targets.
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();

  // Initialize codegen and IR passes used by llc so that the -print-after,
  // -print-before, and -stop-after options work.
  PassRegistry *Registry = PassRegistry::getPassRegistry();
  initializeCore(*Registry);
  initializeCodeGen(*Registry);
  initializeLoopStrengthReducePass(*Registry);
  initializeLowerIntrinsicsPass(*Registry);
  initializeUnreachableBlockElimPass(*Registry);

  // Register the target printer for --version.
  cl::AddExtraVersionPrinter(TargetRegistry::printRegisteredTargetsForVersion);

  //cl::ParseCommandLineOptions(argc, argv, "llvm system compiler\n");

  // Compile the module TimeCompilations times to give better compile time
  // metrics.
  compileModule(module, Context, inputFile);
}

void CodeGenContext::abort()
{
    exit(-1);
}

void CodeGenContext::cloneVariable()
{
}

llvm::Value *CodeGenContext::getVariable(const std::string &id)
{
    llvm::Value *varRef = getVariableRef(id);
    if (NULL == varRef)
    {
        std::cout <<  __FUNCTION__ << " Can't find " << id << std::endl;
        abort();
        return NULL;
    }

    return new LoadInst(varRef, "", false, currentBlock());
}

bool CodeGenContext::isGlobalVariable(const std::string &id)
{
    std::map<std::string, llvm::Value*>::iterator index;
    llvm::Value *varRef = NULL;
    for (CodeGenBlock *block = topBlock; NULL != block; block = block->parent)
    {
        index = block->localRefs.find(id);
        if (index != block->localRefs.end())
        {
            varRef = index->second;
            break;
        }
        index = block->localConstRefs.find(id);
        if (index != block->localConstRefs.end())
        {
            varRef = index->second;
            break;
        }
    }

    if (NULL == varRef)
    {
        index = globalRefs.find(id);
        if (index != globalRefs.end())
        {
            varRef = index->second;
            return true;
        }
        index = globalConstRefs.find(id);
        if (index != globalConstRefs.end())
        {
            varRef = index->second;
            return true;
        }
    }

    return false;
}

bool CodeGenContext::isConstVariable(const std::string &id)
{
    std::map<std::string, llvm::Value*>::iterator index;
    llvm::Value *varRef = NULL;
    bool isConst = false;
    for (CodeGenBlock *block = topBlock; NULL != block; block = block->parent)
    {
        index = block->localRefs.find(id);
        if (index != block->localRefs.end())
        {
            varRef = index->second;
            break;
        }
        index = block->localConstRefs.find(id);
        if (index != block->localConstRefs.end())
        {
            varRef = index->second;
            isConst = true;
            break;
        }
    }

    if (NULL == varRef)
    {
        index = globalRefs.find(id);
        if (index != globalRefs.end())
        {
            varRef = index->second;
            return false;
        }
        index = globalConstRefs.find(id);
        if (index != globalConstRefs.end())
        {
            varRef = index->second;
            return true;
        }
    }

    return isConst;
}

/**
 Find variable in local block first, and then find it in entry block, finally, find it in global
*/
llvm::Value *CodeGenContext::getVariableRef(const std::string &id, bool inLocal)
{
    if (id.length() <= 0)
    {
        return NULL;
    }
    std::map<std::string, llvm::Value*>::iterator index;
    llvm::Value *varRef = NULL;
    for (CodeGenBlock *block = topBlock; NULL != block; block = block->parent)
    {
        index = block->localRefs.find(id);
        if (index != block->localRefs.end())
        {
            varRef = index->second;
            break;
        }
        index = block->localConstRefs.find(id);
        if (index != block->localConstRefs.end())
        {
            varRef = index->second;
            break;
        }
    }

    if (inLocal)
    {
        return varRef;
    }

    if (NULL == varRef)
    {
        index = globalRefs.find(id);
        if (index != globalRefs.end())
        {
            varRef = index->second;
        }
        index = globalConstRefs.find(id);
        if (index != globalConstRefs.end())
        {
            varRef = index->second;
        }
    }

    return varRef;
}

void CodeGenContext::setVariableRef(const std::string &id, llvm::Value *varRef, bool inLocal, bool inConst)
{
    if (id.length() <= 0)
    {
        return ;
    }
    if (inLocal)
    {
        if (inConst)
        {
            topBlock->localConstRefs[id] = varRef;
        }
        else
        {
            topBlock->localRefs[id] = varRef;
        }
    }
    else
    {
        if (inConst)
        {
            globalConstRefs[id] = varRef;
        }
        else
        {
            globalRefs[id] = varRef;
        }
    }
}

llvm::Type *CodeGenContext::getTypeRef(const std::string &name)
{
    llvm::Type *typeRef = globalTypes[name];

    if (NULL == typeRef)
    {
        typeRef = module->getTypeByName(name);
        globalTypes[name] = typeRef;
    }

    return typeRef;
}

void CodeGenContext::setTypeRef(const std::string &name, llvm::Type *typeRef, bool inLocal)
{
    if (inLocal)
    {
        //topBlock->localTypes[name] = varRef;
    }
    else
    {
        globalTypes[name] = typeRef;
    }
}

llvm::Type *CodeGenContext::getObjectType(llvm::Value *object)
{
    if (NULL == object)
    {
        return NULL;
    }

    std::map<llvm::Value*, llvm::Type*>::iterator index = objectTypes.find(object);
    if (index == objectTypes.end())
    {
        return NULL;
    }

    return index->second;
}

void CodeGenContext::setObjectType(llvm::Value *object, llvm::Type *type)
{
    if (NULL == object)
    {
        return ;
    }

    if (NULL == type)
    {
        objectTypes.erase(object);
    }
    else
    {
        objectTypes[object] = type;
    }
}

void CodeGenContext::setBreakBlock(llvm::BasicBlock *block)
{
    if (NULL == topBlock)
    {
        return ;
    }
    topBlock->breakBlock = block;
}

llvm::BasicBlock *CodeGenContext::getBreakBlock()
{
    for (CodeGenBlock *indexBlock = topBlock; NULL != indexBlock; indexBlock = indexBlock->parent)
    {
        if (NULL != indexBlock->breakBlock)
        {
            return indexBlock->breakBlock;
        }
    }

    return NULL;
}

void CodeGenContext::setContinueBlock(llvm::BasicBlock *block)
{
    if (NULL == topBlock)
    {
        return ;
    }
    topBlock->continueBlock = block;
}

llvm::BasicBlock *CodeGenContext::getContinueBlock()
{
    for (CodeGenBlock *indexBlock = topBlock; NULL != indexBlock; indexBlock = indexBlock->parent)
    {
        if (NULL != indexBlock->continueBlock)
        {
            return indexBlock->continueBlock;
        }
    }

    return NULL;
}

void CodeGenContext::pushBlock(llvm::BasicBlock *block, const std::string &name)
{
    topBlock = new CodeGenBlock(topBlock, block, name);
    blocks.push_back(topBlock);
}

void CodeGenContext::popBlock(llvm::BasicBlock *block)
{
    if (NULL == topBlock)
    {
        return;
    }

    do
    {
        if (block == topBlock->block)
        {
            block = NULL;
        }
        blocks.pop_back();
        delete topBlock;
        if (blocks.empty())
        {
            topBlock = NULL;
        }
        else
        {
            topBlock = *(blocks.rbegin());
        }
    }while ((NULL != block) && (NULL != topBlock));
}

llvm::BasicBlock *CodeGenContext::findBlock(const std::string &name)
{
#if 0
    std::vector<CodeGenBlock *>::reverse_iterator rit;
    CodeGenBlock *block;
    for (rit = blocks.rbegin(); rit != blocks.rend(); rit++)
    {
        block = *rit;
        if (0 == name.compare(block->name))
        {
            return block->block;
        }
    }
#else
    for (CodeGenBlock *indexBlock = topBlock; NULL != indexBlock; indexBlock = indexBlock->parent)
    {
        //FIXME: the named block maybe not the lineal predecessor of current block
        //change blocks from stack to vector first, and check from last to front
        if (0 == name.compare(indexBlock->name))
        {
            return indexBlock->block;
        }
    }
#endif

    return NULL;
}

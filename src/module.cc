#include "error.h"
#include "jit_compiler.h"
#include "module.h"

ModuleBuilder::ModuleBuilder(JitCompiler &jit_compiler, const std::string &name)
    : jit_compiler_(jit_compiler),
      context_(std::make_unique<llvm::LLVMContext>()),
      module_(std::make_unique<llvm::Module>(name, *context_)),
      ir_builder_(*context_), debug_info_(*this), codegen_(*this) {}

void ModuleBuilder::Finish() {
  debug_info_.ExitModule();
  jit_compiler_.AddModule(std::move(module_), std::move(context_));
}

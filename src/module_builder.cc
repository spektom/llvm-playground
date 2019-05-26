#include <fstream>

#include "error.h"
#include "jit_compiler.h"
#include "module.h"
#include "module_builder.h"

ModuleBuilder::ModuleBuilder(JitCompiler &jit_compiler, const std::string &name)
    : jit_compiler_(jit_compiler),
      context_(std::make_unique<llvm::LLVMContext>()),
      module_(std::make_unique<llvm::Module>(name, *context_)),
      ir_builder_(*context_), debug_info_(*module_) {}

Module ModuleBuilder::Build() && {
  debug_info_.ExitModule();

  ThrowOnError(jit_compiler_.optimize_layer_.add(
      jit_compiler_.session_.getMainJITDylib(),
      llvm::orc::ThreadSafeModule(std::move(module_), std::move(context_))));

  return Module{jit_compiler_.session_, jit_compiler_.data_layout_};
}

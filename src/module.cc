#include <llvm/Support/raw_os_ostream.h>

#include <stdexcept>

#include "error.h"
#include "jit_compiler.h"
#include "module.h"

ModuleBuilder::ModuleBuilder(JitCompiler &jit_compiler, const std::string &name)
    : jit_compiler_(jit_compiler),
      context_(std::make_unique<llvm::LLVMContext>()),
      module_(std::make_unique<llvm::Module>(name, *context_)),
      ir_builder_(*context_), current_fb_(nullptr), types_(*this),
      constants_(*this) {}

void ModuleBuilder::Finish() {
  if (current_fb_) {
    ExitFunction(nullptr);
  }
  jit_compiler_.AddModule(std::move(module_), std::move(context_));
}

std::string ModuleBuilder::GetIR() const {
  std::string module_str;
  llvm::raw_string_ostream ostream{module_str};
  module_->print(ostream, nullptr, false);
  return module_str;
}

void ModuleBuilder::EnterFunction(
    const std::string &name, llvm::Type *ret_type,
    const std::vector<FunctionBuilder::Argument> &args) {
  if (current_fb_) {
    throw std::runtime_error("Current function is not finished yet!");
  }
  current_fb_.reset(new FunctionBuilder(*this, name, ret_type, args));
}

void ModuleBuilder::ExitFunction(llvm::Value *ret_value) {
  if (!current_fb_) {
    throw std::runtime_error("No function has been started yet!");
  }
  current_fb_->Finish(ret_value);
  current_fb_.reset(nullptr);
}

llvm::Function *ModuleBuilder::RegisterExternalFunction(
    const std::string &name, llvm::Type *ret_type,
    const std::vector<FunctionBuilder::Argument> &args, void *func_ptr) {

  FunctionDeclaration decl(*this, name, ret_type, args);
  jit_compiler_.AddSymbol(name, func_ptr);
  return decl.function();
}

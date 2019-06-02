#include <llvm/Support/raw_os_ostream.h>

#include <iostream>

#include "error.h"
#include "jit_compiler.h"
#include "module.h"

ModuleBuilder::ModuleBuilder(JitCompiler &jit_compiler, const std::string &name)
    : jit_compiler_(jit_compiler),
      context_(std::make_unique<llvm::LLVMContext>()),
      module_(std::make_unique<llvm::Module>(name, *context_)),
      ir_builder_(*context_), types_(*this), constants_(*this),
      statements_(*this) {}

void ModuleBuilder::Build() {
  std::cout << GetIR();
  jit_compiler_.AddModule(std::move(module_), std::move(context_));
}

std::string ModuleBuilder::GetIR() const {
  std::string module_str;
  llvm::raw_string_ostream ostream{module_str};
  module_->print(ostream, nullptr, false);
  return module_str;
}

FuncBuilder &&
ModuleBuilder::CreateFunc(const std::string &name, llvm::Type *ret_type,
                          const std::vector<FuncBuilder::Arg> &args) {
  return std::move(FuncBuilder(*this, name, ret_type, args));
}

llvm::Function *
ModuleBuilder::RegExtFunc(const std::string &name, llvm::Type *ret_type,
                          const std::vector<FuncBuilder::Arg> &args,
                          void *func_ptr) {

  FuncDecl decl(*this, name, ret_type, args);
  jit_compiler_.AddSymbol(name, func_ptr);
  return decl.func();
}

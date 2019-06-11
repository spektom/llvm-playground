#include "llvm/IR/Verifier.h"
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
  Verify();
  jit_compiler_.AddModule(std::move(module_), std::move(context_));
}

std::string ModuleBuilder::GetIR() const {
  std::string module_str;
  llvm::raw_string_ostream ostream{module_str};
  module_->print(ostream, nullptr, false);
  return module_str;
}

void ModuleBuilder::Verify() const {
  std::string error_str;
  llvm::raw_string_ostream ostream{error_str};
  if (llvm::verifyModule(*module_, &ostream)) {
    std::cout << "ERROR found in IR: " << GetIR() << "\nERROR: " << error_str
              << "\n\n";
  }
}

FuncBuilder &&
ModuleBuilder::GetFunc(const std::string &name, llvm::Type *ret_type,
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

Struct &&ModuleBuilder::GetStruct(const std::string &name,
                                  const std::vector<Struct::Member> &members) {
  return std::move(Struct(*this, name, members));
}

Struct &&ModuleBuilder::GetStruct(llvm::Value *ptr, const std::string &name,
                                  const std::vector<Struct::Member> &members) {
  return std::move(Struct(*this, ptr, name, members));
}

Vector &&ModuleBuilder::GetVector(llvm::Type *element_type, uint32_t size) {
  return std::move(Vector(*this, element_type, size));
}

Vector &&ModuleBuilder::GetVector(llvm::Value *ptr, llvm::Type *element_type,
                                  uint32_t size) {
  return std::move(Vector(*this, ptr, element_type, size));
}

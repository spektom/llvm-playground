#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <string>

#include "jit_compiler.h"

class CodeGen {
public:
  CodeGen(const std::string &module_name,
          OptimizationLevel optimization_level = OptimizationLevel::O2);

private:
  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  std::unique_ptr<llvm::IRBuilder<>> builder_;
  std::unique_ptr<JitCompiler> jit_;

  // Commonly used types:
  llvm::Type *bool_type_;
  llvm::Type *int8_type_;
  llvm::Type *int16_type_;
  llvm::Type *int32_type_;
  llvm::Type *int64_type_;
  llvm::Type *float_type_;
  llvm::Type *double_type_;
  llvm::Type *void_type_;
  llvm::Type *void_ptr_type_;
  llvm::PointerType *char_ptr_type_;
};

#endif /* CODEGEN_H_ */

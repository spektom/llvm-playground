#ifndef MODULE_H_
#define MODULE_H_

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "constants.h"
#include "function.h"
#include "macros.h"
#include "types.h"

class JitCompiler;

class ModuleBuilder {
public:
  ModuleBuilder(JitCompiler &, const std::string &);
  DISALLOW_COPY_AND_MOVE(ModuleBuilder);

  void Finish();
  std::string GetIR() const;

  llvm::LLVMContext &context() const { return *context_; }
  llvm::Module &module() const { return *module_; }
  llvm::IRBuilder<> &ir_builder() { return ir_builder_; }
  Types &types() { return types_; }
  Constants &constants() { return constants_; }

  void EnterFunction(const std::string &, llvm::Type *,
                     const std::vector<FunctionBuilder::Argument> &);
  void ExitFunction(llvm::Value *);
  llvm::Function *
  RegisterExternalFunction(const std::string &, llvm::Type *,
                           const std::vector<FunctionBuilder::Argument> &,
                           void *);

private:
  JitCompiler &jit_compiler_;
  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  llvm::IRBuilder<> ir_builder_;
  std::unique_ptr<FunctionBuilder> current_fb_;
  Types types_;
  Constants constants_;
};

#endif /* MODULE_H_ */

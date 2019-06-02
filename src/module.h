#ifndef MODULE_H_
#define MODULE_H_

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "constants.h"
#include "function.h"
#include "macros.h"
#include "statements.h"
#include "types.h"

class JitCompiler;

class ModuleBuilder {
public:
  ModuleBuilder(JitCompiler &, const std::string &);
  DISALLOW_COPY_AND_MOVE(ModuleBuilder);

  void Build();
  std::string GetIR() const;

  llvm::LLVMContext &context() const { return *context_; }
  llvm::Module &module() const { return *module_; }
  llvm::IRBuilder<> &ir_builder() { return ir_builder_; }
  Types &types() { return types_; }
  Constants &constants() { return constants_; }
  Statements &statements() { return statements_; }

  FuncBuilder &&CreateFunc(const std::string &, llvm::Type *,
                           const std::vector<FuncBuilder::Arg> &);
  llvm::Function *RegExtFunc(const std::string &, llvm::Type *,
                             const std::vector<FuncBuilder::Arg> &, void *);

private:
  JitCompiler &jit_compiler_;
  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  llvm::IRBuilder<> ir_builder_;
  Types types_;
  Constants constants_;
  Statements statements_;
};

#endif /* MODULE_H_ */

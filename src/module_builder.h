#ifndef MODULE_BUILDER_H_
#define MODULE_BUILDER_H_

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "debug_info.h"
#include "macros.h"

class JitCompiler;
class Module;

class ModuleBuilder {
  friend class FunctionBuilder;

public:
  ModuleBuilder(JitCompiler &, const std::string &name);
  DISALLOW_COPY_AND_MOVE(ModuleBuilder);

  template <typename FunctionType, typename FunctionBody>
  auto CreateFunction(std::string const &name, FunctionBody &&fb);

  [[nodiscard]] Module Build() &&;

private:
  JitCompiler &jit_compiler_;
  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  llvm::IRBuilder<> ir_builder_;
  DebugInfo debug_info_;
};

#endif /* MODULE_BUILDER_H_ */

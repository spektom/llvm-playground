#ifndef MODULE_H_
#define MODULE_H_

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "codegen.h"
#include "debug_info.h"
#include "macros.h"

class JitCompiler;

class ModuleBuilder {
public:
  ModuleBuilder(JitCompiler &, const std::string &);
  DISALLOW_COPY_AND_MOVE(ModuleBuilder);

  void Finish();

  llvm::LLVMContext &context() const { return *context_; }
  llvm::Module &module() const { return *module_; }
  llvm::IRBuilder<> &ir_builder() { return ir_builder_; }
  DebugInfo &debug_info() { return debug_info_; }
  CodeGen &codegen() { return codegen_; }

private:
  JitCompiler &jit_compiler_;
  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  llvm::IRBuilder<> ir_builder_;
  DebugInfo debug_info_;
  CodeGen codegen_;
};

#endif /* MODULE_H_ */

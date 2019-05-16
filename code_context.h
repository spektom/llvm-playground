#ifndef CODE_CONTEXT_H_
#define CODE_CONTEXT_H_

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <string>

#include "optimization_level.h"

class CodeContext {
public:
  CodeContext(const std::string &module_name,
              OptimizationLevel optimization_level);

private:
  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  std::unique_ptr<llvm::IRBuilder<>> builder_;
};

#endif /* CODE_CONTEXT_H_ */

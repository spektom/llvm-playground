#include "code_context.h"

CodeContext::CodeContext(const std::string &module_name,
                         OptimizationLevel optimization_level)
    : context_(std::make_unique<llvm::LLVMContext>()),
      module_(std::make_unique<llvm::Module>(module_name, *context_)),
      builder_(std::make_unique<llvm::IRBuilder<>>(*context_)) {}

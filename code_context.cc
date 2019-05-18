#include "code_context.h"

CodeContext::CodeContext(const std::string &module_name,
                         OptimizationLevel optimization_level)
    : context_(std::make_unique<llvm::LLVMContext>()),
      module_(std::make_unique<llvm::Module>(module_name, *context_)),
      builder_(std::make_unique<llvm::IRBuilder<>>(*context_)) {

  // Commonly used types
  bool_type_ = llvm::Type::getInt1Ty(*context_);
  int8_type_ = llvm::Type::getInt8Ty(*context_);
  int16_type_ = llvm::Type::getInt16Ty(*context_);
  int32_type_ = llvm::Type::getInt32Ty(*context_);
  int64_type_ = llvm::Type::getInt64Ty(*context_);
  double_type_ = llvm::Type::getDoubleTy(*context_);
  float_type_ = llvm::Type::getFloatTy(*context_);
  void_type_ = llvm::Type::getVoidTy(*context_);
  void_ptr_type_ = llvm::Type::getInt8PtrTy(*context_);
  char_ptr_type_ = llvm::Type::getInt8PtrTy(*context_);
}

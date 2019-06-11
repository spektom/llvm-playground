#include "types.h"
#include "module.h"

Types::Types(ModuleBuilder &mb) : mb_(mb) {
  bool_ = llvm::Type::getInt1Ty(mb_.context());
  int8_ = llvm::Type::getInt8Ty(mb_.context());
  int16_ = llvm::Type::getInt16Ty(mb_.context());
  int32_ = llvm::Type::getInt32Ty(mb_.context());
  int64_ = llvm::Type::getInt64Ty(mb_.context());
  double_ = llvm::Type::getDoubleTy(mb_.context());
  float_ = llvm::Type::getFloatTy(mb_.context());
  void_ = llvm::Type::getVoidTy(mb_.context());
  int8_ptr_ = llvm::Type::getInt8PtrTy(mb_.context());
}

llvm::Type *Types::Array(llvm::Type *type, uint32_t num_elements) const {
  return llvm::ArrayType::get(type, num_elements);
}

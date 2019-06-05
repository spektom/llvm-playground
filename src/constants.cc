#include <llvm/IR/Type.h>

#include "constants.h"
#include "module.h"

Constants::Constants(ModuleBuilder &mb) : mb_(mb) {}

llvm::Constant *Constants::Get(bool val) const {
  if (val) {
    return llvm::ConstantInt::getTrue(mb_.context());
  } else {
    return llvm::ConstantInt::getFalse(mb_.context());
  }
}

llvm::Constant *Constants::Get(int8_t val) const {
  return llvm::ConstantInt::get(mb_.types().Int8(), val, true);
}

llvm::Constant *Constants::Get(int16_t val) const {
  return llvm::ConstantInt::get(mb_.types().Int16(), val, true);
}

llvm::Constant *Constants::Get(int32_t val) const {
  return llvm::ConstantInt::get(mb_.types().Int32(), val, true);
}

llvm::Constant *Constants::Get(int64_t val) const {
  return llvm::ConstantInt::get(mb_.types().Int64(), val, true);
}

llvm::Constant *Constants::Get(uint8_t val) const {
  return llvm::ConstantInt::get(mb_.types().Int8(), val, false);
}

llvm::Constant *Constants::Get(uint16_t val) const {
  return llvm::ConstantInt::get(mb_.types().Int16(), val, false);
}

llvm::Constant *Constants::Get(uint32_t val) const {
  return llvm::ConstantInt::get(mb_.types().Int32(), val, false);
}

llvm::Constant *Constants::Get(uint64_t val) const {
  return llvm::ConstantInt::get(mb_.types().Int64(), val, false);
}

llvm::Constant *Constants::Get(float val) const {
  return llvm::ConstantFP::get(mb_.types().Float(), val);
}

llvm::Constant *Constants::Get(double val) const {
  return llvm::ConstantFP::get(mb_.types().Double(), val);
}

llvm::Value *Constants::Get(const std::string &str) const {
  return llvm::ConstantDataArray::getString(mb_.context(), str);
}

llvm::Constant *Constants::Null(llvm::Type *type) const {
  return llvm::Constant::getNullValue(type);
}

llvm::Constant *Constants::NullPtr(llvm::PointerType *type) const {
  return llvm::ConstantPointerNull::get(type);
}

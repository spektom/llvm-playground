#include "codegen.h"
#include "module.h"

CodeGen::CodeGen(ModuleBuilder &mb) : mb_(mb) {
  bool_type_ = llvm::Type::getInt1Ty(mb_.context());
  int8_type_ = llvm::Type::getInt8Ty(mb_.context());
  int16_type_ = llvm::Type::getInt16Ty(mb_.context());
  int32_type_ = llvm::Type::getInt32Ty(mb_.context());
  int64_type_ = llvm::Type::getInt64Ty(mb_.context());
  double_type_ = llvm::Type::getDoubleTy(mb_.context());
  float_type_ = llvm::Type::getFloatTy(mb_.context());
  void_type_ = llvm::Type::getVoidTy(mb_.context());
  void_ptr_type_ = llvm::Type::getInt8PtrTy(mb_.context());
  char_ptr_type_ = llvm::Type::getInt8PtrTy(mb_.context());
}

llvm::Type *CodeGen::ArrayType(llvm::Type *type, uint32_t num_elements) const {
  return llvm::ArrayType::get(type, num_elements);
}

/// Constant wrappers for bool, int8, int16, int32, int64, strings and NULL
llvm::Constant *CodeGen::ConstBool(bool val) const {
  if (val) {
    return llvm::ConstantInt::getTrue(mb_.context());
  } else {
    return llvm::ConstantInt::getFalse(mb_.context());
  }
}

llvm::Constant *CodeGen::Const8(int8_t val) const {
  return llvm::ConstantInt::get(Int8Type(), val, true);
}

llvm::Constant *CodeGen::Const16(int16_t val) const {
  return llvm::ConstantInt::get(Int16Type(), val, true);
}

llvm::Constant *CodeGen::Const32(int32_t val) const {
  return llvm::ConstantInt::get(Int32Type(), val, true);
}

llvm::Constant *CodeGen::Const64(int64_t val) const {
  return llvm::ConstantInt::get(Int64Type(), val, true);
}

llvm::Constant *CodeGen::ConstDouble(double val) const {
  return llvm::ConstantFP::get(DoubleType(), val);
}

llvm::Value *CodeGen::ConstString(const std::string &str_val,
                                  const std::string &name) const {
  // Strings are treated as arrays of bytes
  auto *str = llvm::ConstantDataArray::getString(mb_.context(), str_val);
  auto *global_var =
      new llvm::GlobalVariable(mb_.module(), str->getType(), true,
                               llvm::GlobalValue::InternalLinkage, str, name);
  return mb_.ir_builder().CreateInBoundsGEP(global_var,
                                            {Const32(0), Const32(0)});
}

llvm::Value *CodeGen::ConstGenericBytes(const void *data, uint32_t length,
                                        const std::string &name) const {
  // Create the constant data array that wraps the input data
  llvm::ArrayRef<uint8_t> elements{reinterpret_cast<const uint8_t *>(data),
                                   length};
  auto *arr = llvm::ConstantDataArray::get(mb_.context(), elements);

  // Create a global variable for the data
  auto *global_var =
      new llvm::GlobalVariable(mb_.module(), arr->getType(), true,
                               llvm::GlobalValue::InternalLinkage, arr, name);

  // Return a pointer to the first element
  return mb_.ir_builder().CreateInBoundsGEP(global_var,
                                            {Const32(0), Const32(0)});
}

llvm::Constant *CodeGen::Null(llvm::Type *type) const {
  return llvm::Constant::getNullValue(type);
}

llvm::Constant *CodeGen::NullPtr(llvm::PointerType *type) const {
  return llvm::ConstantPointerNull::get(type);
}

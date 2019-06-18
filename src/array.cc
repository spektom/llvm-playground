#include <llvm/IR/Instructions.h>

#include "array.h"
#include "jit_compiler.h"
#include "module.h"

Array::Array(ModuleBuilder &mb, llvm::Type *element_type, uint32_t size)
    : mb_(mb), ptr_(nullptr), element_type_(element_type), size_(size) {
  Allocate();
}

Array::Array(ModuleBuilder &mb, llvm::Value *ptr)
    : mb_(mb), ptr_(ptr), element_type_(nullptr) {}

llvm::Value *Array::Get(uint32_t index) {
  return Get(mb_.constants().Get(index));
}

llvm::Value *Array::GetPtr(uint32_t index) {
  return GetPtr(mb_.constants().Get(index));
}

llvm::Value *Array::GetPtr(llvm::Value *index) {
  return mb_.ir_builder().CreateInBoundsGEP(
      ptr_, {mb_.constants().Get((uint32_t)0), index});
}

llvm::Value *Array::Get(llvm::Value *index) {
  return mb_.ir_builder().CreateLoad(GetPtr(index));
}

void Array::Set(uint32_t index, llvm::Value *value) {
  Set(mb_.constants().Get(index), value);
}

void Array::Set(llvm::Value *index, llvm::Value *value) {
  mb_.ir_builder().CreateStore(value, GetPtr(index));
}

void Array::Allocate() {
  ptr_ =
      mb_.ir_builder().CreateAlloca(llvm::ArrayType::get(element_type_, size_));
}

#include <llvm/IR/Instructions.h>

#include "jit_compiler.h"
#include "module.h"
#include "vector.h"

Vector::Vector(ModuleBuilder &mb, llvm::Type *element_type, uint32_t size)
    : mb_(mb), ptr_(nullptr), element_type_(element_type), size_(size) {
  Allocate();
}

Vector::Vector(ModuleBuilder &mb, llvm::Value *ptr)
    : mb_(mb), ptr_(ptr), element_type_(nullptr) {}

llvm::Value *Vector::Get(uint32_t index) {
  return Get(mb_.constants().Get(index));
}

llvm::Value *Vector::GetPtr(uint32_t index) {
  return GetPtr(mb_.constants().Get(index));
}

llvm::Value *Vector::GetPtr(llvm::Value *index) {
  return mb_.ir_builder().CreateInBoundsGEP(ptr_, index);
}

llvm::Value *Vector::Get(llvm::Value *index) {
  return mb_.ir_builder().CreateLoad(GetPtr(index));
}

void Vector::Set(uint32_t index, llvm::Value *value) {
  Set(mb_.constants().Get(index), value);
}

void Vector::Set(llvm::Value *index, llvm::Value *value) {
  mb_.ir_builder().CreateStore(value, GetPtr(index));
}

void Vector::Allocate() {
  auto &data_layout = mb_.jit_compiler().data_layout();

  auto alloc_size = llvm::ConstantExpr::getMul(
      mb_.constants().Get(data_layout.getTypeAllocSize(element_type_)),
      mb_.constants().Get(size_));

  auto malloc_inst = llvm::CallInst::CreateMalloc(
      mb_.ir_builder().GetInsertBlock(),
      data_layout.getIntPtrType(mb_.context()), element_type_, alloc_size,
      nullptr, nullptr, "");
  mb_.ir_builder().Insert(malloc_inst);
  ptr_ = malloc_inst;
}

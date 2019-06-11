#ifndef VECTOR_H_
#define VECTOR_H_

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "macros.h"

class ModuleBuilder;

class Vector {
public:
  Vector(ModuleBuilder &, llvm::Type *, uint32_t);
  Vector(ModuleBuilder &, llvm::Value *, llvm::Type *, uint32_t);
  ~Vector() {}

  llvm::Value *ptr() const { return ptr_; }

  llvm::Value *Get(uint32_t);
  llvm::Value *Get(llvm::Value *);
  void Set(uint32_t, llvm::Value *);
  void Set(llvm::Value *, llvm::Value *);

private:
  void Allocate();

private:
  ModuleBuilder &mb_;
  llvm::Type *element_type_;
  uint32_t size_;
  llvm::Value *ptr_;
};

#endif /* VECTOR_H_ */

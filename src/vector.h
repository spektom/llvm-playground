#ifndef VECTOR_H_
#define VECTOR_H_

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "macros.h"

class ModuleBuilder;

// Dynamically allocated array
class Vector {
public:
  Vector(ModuleBuilder &, llvm::Type *, uint32_t);
  Vector(ModuleBuilder &, llvm::Value *);
  ~Vector() {}

  llvm::Value *ptr() const { return ptr_; }

  llvm::Value *Get(uint32_t);
  llvm::Value *Get(llvm::Value *);
  llvm::Value *GetPtr(uint32_t);
  llvm::Value *GetPtr(llvm::Value *);
  void Set(uint32_t, llvm::Value *);
  void Set(llvm::Value *, llvm::Value *);

private:
  void Allocate();

private:
  ModuleBuilder &mb_;
  llvm::Value *ptr_;
  llvm::Type *element_type_;
  uint32_t size_;
};

#endif /* VECTOR_H_ */

#ifndef ARRAY_H_
#define ARRAY_H_

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "macros.h"

class ModuleBuilder;

// Statically sized array
class Array {
public:
  Array(ModuleBuilder &, llvm::Type *, uint32_t);
  Array(ModuleBuilder &, llvm::Value *);
  ~Array() {}

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

#endif /* ARRAY_H_ */

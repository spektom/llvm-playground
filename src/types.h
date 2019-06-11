#ifndef TYPES_H_
#define TYPES_H_

#include "llvm/IR/Type.h"

#include "macros.h"

class ModuleBuilder;

class Types {
public:
  Types(ModuleBuilder &mb);
  DISALLOW_COPY_AND_MOVE(Types);

  llvm::Type *Bool() const { return bool_; }
  llvm::Type *Int8() const { return int8_; }
  llvm::Type *Int16() const { return int16_; }
  llvm::Type *Int32() const { return int32_; }
  llvm::Type *Int64() const { return int64_; }
  llvm::Type *Float() const { return float_; }
  llvm::Type *Double() const { return double_; }
  llvm::Type *Void() const { return void_; }
  llvm::PointerType *VoidPtr() const { return int8_ptr_; }
  llvm::PointerType *CharPtr() const { return int8_ptr_; }
  llvm::PointerType *Int8Ptr() const { return int8_ptr_; }
  llvm::Type *Array(llvm::Type *, uint32_t) const;

private:
  ModuleBuilder &mb_;

  llvm::Type *bool_;
  llvm::Type *int8_;
  llvm::Type *int16_;
  llvm::Type *int32_;
  llvm::Type *int64_;
  llvm::Type *float_;
  llvm::Type *double_;
  llvm::Type *void_;
  llvm::PointerType *int8_ptr_;
};

#endif /* TYPES_H_ */

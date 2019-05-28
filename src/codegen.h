#ifndef CODEGEN_H_
#define CODEGEN_H_

#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

#include "macros.h"

class ModuleBuilder;

class CodeGen {
public:
  CodeGen(ModuleBuilder &mb);
  DISALLOW_COPY_AND_MOVE(CodeGen);

  llvm::Type *BoolType() const { return bool_type_; }
  llvm::Type *Int8Type() const { return int8_type_; }
  llvm::Type *Int16Type() const { return int16_type_; }
  llvm::Type *Int32Type() const { return int32_type_; }
  llvm::Type *Int64Type() const { return int64_type_; }
  llvm::Type *FloatType() const { return float_type_; }
  llvm::Type *DoubleType() const { return double_type_; }
  llvm::Type *VoidType() const { return void_type_; }
  llvm::Type *VoidPtrType() const { return void_ptr_type_; }
  llvm::PointerType *CharPtrType() const { return char_ptr_type_; }
  llvm::Type *ArrayType(llvm::Type *, uint32_t) const;

  llvm::Constant *ConstBool(bool) const;
  llvm::Constant *Const8(int8_t) const;
  llvm::Constant *Const16(int16_t) const;
  llvm::Constant *Const32(int32_t) const;
  llvm::Constant *Const64(int64_t) const;
  llvm::Constant *ConstDouble(double) const;
  llvm::Value *ConstString(const std::string &, const std::string &) const;
  llvm::Value *ConstGenericBytes(const void *, uint32_t,
                                 const std::string &) const;
  llvm::Constant *Null(llvm::Type *) const;
  llvm::Constant *NullPtr(llvm::PointerType *) const;

private:
  ModuleBuilder &mb_;

  llvm::Type *bool_type_;
  llvm::Type *int8_type_;
  llvm::Type *int16_type_;
  llvm::Type *int32_type_;
  llvm::Type *int64_type_;
  llvm::Type *float_type_;
  llvm::Type *double_type_;
  llvm::Type *void_type_;
  llvm::Type *void_ptr_type_;
  llvm::PointerType *char_ptr_type_;
};

#endif /* CODEGEN_H_ */

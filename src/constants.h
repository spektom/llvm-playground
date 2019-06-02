#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <llvm/IR/Constant.h>

#include "macros.h"

class ModuleBuilder;

class Constants {
public:
  Constants(ModuleBuilder &mb);
  DISALLOW_COPY_AND_MOVE(Constants);

  llvm::Constant *Get(bool) const;
  llvm::Constant *Get(int8_t) const;
  llvm::Constant *Get(int16_t) const;
  llvm::Constant *Get(int32_t) const;
  llvm::Constant *Get(int64_t) const;
  llvm::Constant *Get(float) const;
  llvm::Constant *Get(double) const;
  llvm::Value *Get(const std::string &) const;
  llvm::Constant *Null(llvm::Type *) const;
  llvm::Constant *NullPtr(llvm::PointerType *) const;

private:
  ModuleBuilder &mb_;
};

#endif /* CONSTANTS_H_ */

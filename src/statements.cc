#include "statements.h"
#include "module.h"

Statements::Statements(ModuleBuilder &mb) : mb_(mb) {}

llvm::Value *Statements::Call(llvm::Value *function,
                              const std::vector<llvm::Value *> &args) {
  return mb_.ir_builder().CreateCall(function, args);
}

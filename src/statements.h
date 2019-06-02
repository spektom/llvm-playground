#ifndef STATEMENTS_H_
#define STATEMENTS_H_

#include <llvm/IR/Value.h>

#include "macros.h"

class ModuleBuilder;

class Statements {
public:
  Statements(ModuleBuilder &mb);
  DISALLOW_COPY_AND_MOVE(Statements);

  llvm::Value *Call(llvm::Value *, const std::vector<llvm::Value *> &);

private:
  ModuleBuilder &mb_;
};

#endif /* STATEMENTS_H_ */

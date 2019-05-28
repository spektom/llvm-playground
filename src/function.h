#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

#include "macros.h"

class ModuleBuilder;

class FunctionBuilder {
public:
  struct Argument {
    std::string name;
    llvm::Type *type;
    Argument(std::string _name, llvm::Type *_type)
        : name(std::move(_name)), type(_type) {}
  };

public:
  FunctionBuilder(ModuleBuilder &, const std::string &, llvm::Type *,
                  const std::vector<Argument> &);
  DISALLOW_COPY_AND_MOVE(FunctionBuilder);

  llvm::Value *GetArgumentByName(const std::string &);
  llvm::Value *GetArgumentByPosition(uint32_t);
  void Finish(llvm::Value *);

  llvm::Function *function() const { return function_; }

private:
  ModuleBuilder &mb_;
  std::string name_;
  llvm::Function *function_;
};

#endif /* FUNCTION_H_ */

#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

#include "macros.h"

class ModuleBuilder;

class FunctionDeclaration {
public:
  struct Argument {
    std::string name;
    llvm::Type *type;
    Argument(std::string _name, llvm::Type *_type)
        : name(std::move(_name)), type(_type) {}
  };

public:
  FunctionDeclaration(ModuleBuilder &, const std::string &, llvm::Type *,
                      const std::vector<Argument> &);
  DISALLOW_COPY_AND_MOVE(FunctionDeclaration);
  ~FunctionDeclaration() {}

  llvm::Value *GetArgumentByName(const std::string &);
  llvm::Value *GetArgumentByPosition(uint32_t);

  llvm::Function *function() const { return function_; }

private:
  std::string name_;
  llvm::Function *function_;
};

class FunctionBuilder : public FunctionDeclaration {
public:
  FunctionBuilder(ModuleBuilder &, const std::string &, llvm::Type *,
                  const std::vector<Argument> &);
  DISALLOW_COPY_AND_MOVE(FunctionBuilder);
  ~FunctionBuilder() {}

  void Finish(llvm::Value *);

private:
  ModuleBuilder &mb_;
};

#endif /* FUNCTION_H_ */

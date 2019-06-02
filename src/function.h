#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

#include "macros.h"

class ModuleBuilder;

class FuncDecl {
public:
  struct Arg {
    std::string name;
    llvm::Type *type;
    Arg(const std::string &_name, llvm::Type *_type)
        : name(_name), type(_type) {}

    Arg(llvm::Type *_type) : Arg("", _type) {}
  };

public:
  FuncDecl(ModuleBuilder &, const std::string &, llvm::Type *,
           const std::vector<Arg> &);
  ~FuncDecl() {}

  llvm::Value *GetArgByName(const std::string &);
  llvm::Function *func() const { return func_; }

private:
  llvm::Function *func_;
};

class FuncBuilder : public FuncDecl {
public:
  FuncBuilder(ModuleBuilder &, const std::string &, llvm::Type *,
              const std::vector<Arg> &);
  ~FuncBuilder() {}

  void Return(llvm::Value *ret_val = nullptr);

private:
  ModuleBuilder &mb_;
};

#endif /* FUNCTION_H_ */

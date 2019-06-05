#ifndef STRUCT_H_
#define STRUCT_H_

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <unordered_map>

#include "macros.h"

class ModuleBuilder;

class Struct {
public:
  struct Member {
    std::string name;
    llvm::Type *type;
    Member(const std::string &_name, llvm::Type *_type)
        : name(_name), type(_type) {}
  };

public:
  Struct(ModuleBuilder &, const std::string &, const std::vector<Member> &);
  ~Struct() {}

  llvm::Value *Get(llvm::Value *, const std::string &);
  void Set(llvm::Value *, const std::string &, llvm::Value *);
  llvm::Value *New();

private:
  ModuleBuilder &mb_;
  std::string name_;
  llvm::StructType *struct_;
  std::unordered_map<std::string, uint32_t> member_indices_;
};

#endif /* STRUCT_H_ */

#ifndef STRUCT_H_
#define STRUCT_H_

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <unordered_map>

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
  Struct(ModuleBuilder &, llvm::Value *, const std::string &,
         const std::vector<Member> &);

  ~Struct() {}

  llvm::Value *ptr() { return ptr_; }

  llvm::Value *GetPtr(const std::string &);
  llvm::Value *Get(const std::string &);
  void Set(const std::string &, llvm::Value *);

private:
  void Allocate();

private:
  ModuleBuilder &mb_;
  llvm::Value *ptr_;
  std::string name_;
  llvm::StructType *struct_type_;
  std::unordered_map<std::string, uint32_t> member_indices_;
};

#endif /* STRUCT_H_ */

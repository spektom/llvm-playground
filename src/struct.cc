#include <llvm/IR/Instructions.h>

#include "jit_compiler.h"
#include "module.h"
#include "struct.h"
#include <iostream>

Struct::Struct(ModuleBuilder &mb, const std::string &name,
               const std::vector<Member> &members)
    : Struct(mb, nullptr, name, members) {
  Allocate();
}

Struct::Struct(ModuleBuilder &mb, llvm::Value *ptr, const std::string &name,
               const std::vector<Member> &members)
    : mb_(mb), name_(name), ptr_(ptr) {

  std::vector<llvm::Type *> member_types;
  member_types.reserve(members.size());
  uint32_t idx = 0;
  for (auto &member : members) {
    member_indices_.insert(std::make_pair(member.name, idx++));
    member_types.push_back(member.type);
  }

  struct_type_ = llvm::StructType::create(mb_.context(), name);
  struct_type_->setBody(member_types, false /* packed */);
}

llvm::Value *Struct::Get(const std::string &member) {
  auto member_ptr =
      mb_.ir_builder().CreateStructGEP(ptr_, member_indices_[member]);
  return mb_.ir_builder().CreateLoad(member_ptr);
}

void Struct::Set(const std::string &member, llvm::Value *value) {
  auto member_ptr =
      mb_.ir_builder().CreateStructGEP(ptr_, member_indices_[member]);
  mb_.ir_builder().CreateStore(value, member_ptr);
}

void Struct::Allocate() {
  auto &data_layout = mb_.jit_compiler().data_layout();

  auto struct_size = mb_.constants().Get(
      data_layout.getStructLayout(struct_type_)->getSizeInBytes());

  auto malloc_inst = llvm::CallInst::CreateMalloc(
      mb_.ir_builder().GetInsertBlock(),
      data_layout.getIntPtrType(mb_.context()), struct_type_, struct_size,
      nullptr, nullptr, "");
  mb_.ir_builder().Insert(malloc_inst);
  ptr_ = malloc_inst;
}

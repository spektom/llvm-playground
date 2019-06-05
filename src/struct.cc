#include <llvm/IR/Instructions.h>

#include "jit_compiler.h"
#include "module.h"
#include "struct.h"

Struct::Struct(ModuleBuilder &mb, const std::string &name,
               const std::vector<Member> &members)
    : mb_(mb), name_(name) {

  std::vector<llvm::Type *> member_types(members.size());
  uint32_t idx = 0;
  for (auto &member : members) {
    member_indices_.insert(std::make_pair(member.name, idx++));
    member_types.push_back(member.type);
  }

  struct_ = llvm::StructType::create(mb.context(), name);
  struct_->setBody(member_types, false /* packed */);
}

llvm::Value *Struct::Get(llvm::Value *struct_obj, const std::string &member) {
  auto member_ptr =
      mb_.ir_builder().CreateStructGEP(struct_obj, member_indices_[member]);
  return mb_.ir_builder().CreateLoad(member_ptr);
}

void Struct::Set(llvm::Value *struct_obj, const std::string &member,
                 llvm::Value *value) {
  auto member_ptr =
      mb_.ir_builder().CreateStructGEP(struct_obj, member_indices_[member]);
  mb_.ir_builder().CreateStore(value, member_ptr);
}

llvm::Value *Struct::New() {
  auto &data_layout = mb_.jit_compiler().data_layout();

  // sizeof(Struct)
  auto struct_size = mb_.constants().Get(
      data_layout.getStructLayout(struct_)->getSizeInBytes());

  // sizeof(*Struct)
  auto struct_ptr_type = llvm::PointerType::get(struct_, 0);

  // malloc()
  auto malloc_inst = llvm::CallInst::CreateMalloc(
      mb_.ir_builder().GetInsertBlock(), struct_ptr_type, struct_, struct_size,
      nullptr, nullptr, "");

  // cast to Struct*
  return mb_.ir_builder().CreateBitCast(malloc_inst, struct_ptr_type);
}

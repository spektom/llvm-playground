#include "function.h"
#include "module.h"

FuncDecl::FuncDecl(ModuleBuilder &mb, const std::string &name,
                   llvm::Type *ret_type, const std::vector<Arg> &args) {
  std::vector<llvm::Type *> arg_types;
  for (auto &arg : args) {
    arg_types.push_back(arg.type);
  }

  auto *fn_type = llvm::FunctionType::get(ret_type, arg_types, false);
  func_ = llvm::Function::Create(
      fn_type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name,
      &mb.module());

  auto arg_iter = args.begin();
  for (auto iter = func_->arg_begin(), end = func_->arg_end(); iter != end;
       iter++, arg_iter++) {
    iter->setName(arg_iter->name);
  }
}

llvm::Value *FuncDecl::GetArgByName(const std::string &name) {
  for (auto &arg : func_->args()) {
    if (arg.getName().equals(name)) {
      return &arg;
    }
  }
  return nullptr;
}

FuncBuilder::FuncBuilder(ModuleBuilder &mb, const std::string &name,
                         llvm::Type *ret_type, const std::vector<Arg> &args)
    : FuncDecl(mb, name, ret_type, args), mb_(mb) {

  auto fn = func();

  auto block = llvm::BasicBlock::Create(mb_.context(), "entry", fn);
  mb_.ir_builder().SetInsertPoint(block);

  fn->addFnAttr("target-cpu", llvm::sys::getHostCPUName());
}

void FuncBuilder::Return(llvm::Value *ret_value) {
  if (ret_value != nullptr) {
    mb_.ir_builder().CreateRet(ret_value);
  } else {
    mb_.ir_builder().CreateRetVoid();
  }
}

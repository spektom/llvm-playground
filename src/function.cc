#include "function.h"
#include "module.h"

FunctionDeclaration::FunctionDeclaration(ModuleBuilder &mb,
                                         const std::string &name,
                                         llvm::Type *ret_type,
                                         const std::vector<Argument> &args)
    : name_(name) {

  std::vector<llvm::Type *> arg_types;
  for (auto &arg : args) {
    arg_types.push_back(arg.type);
  }

  auto *fn_type = llvm::FunctionType::get(ret_type, arg_types, false);
  function_ = llvm::Function::Create(
      fn_type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name,
      &mb.module());

  auto arg_iter = args.begin();
  for (auto iter = function_->arg_begin(), end = function_->arg_end();
       iter != end; iter++, arg_iter++) {
    iter->setName(arg_iter->name);
  }
}

llvm::Value *FunctionDeclaration::GetArgumentByName(const std::string &name) {
  for (auto &arg : function_->args()) {
    if (arg.getName().equals(name)) {
      return &arg;
    }
  }
  return nullptr;
}

llvm::Value *FunctionDeclaration::GetArgumentByPosition(uint32_t index) {
  uint32_t pos = 0;
  for (auto arg_iter = function_->arg_begin(), end = function_->arg_end();
       arg_iter != end; ++arg_iter, ++pos) {
    if (pos == index) {
      return &*arg_iter;
    }
  }
  return nullptr;
}

FunctionBuilder::FunctionBuilder(ModuleBuilder &mb, const std::string &name,
                                 llvm::Type *ret_type,
                                 const std::vector<Argument> &args)
    : FunctionDeclaration(mb, name, ret_type, args), mb_(mb) {

  auto func = function();

  auto block = llvm::BasicBlock::Create(mb_.context(), "entry", func);
  mb_.ir_builder().SetInsertPoint(block);

  func->addFnAttr("target-cpu", llvm::sys::getHostCPUName());
}

void FunctionBuilder::Finish(llvm::Value *ret_value) {
  if (ret_value != nullptr) {
    mb_.ir_builder().CreateRet(ret_value);
  } else {
    mb_.ir_builder().CreateRetVoid();
  }
}

#include "function.h"
#include "module.h"

FunctionBuilder::FunctionBuilder(ModuleBuilder &mb, const std::string &name,
                                 llvm::Type *ret_type,
                                 const std::vector<Argument> &args)
    : mb_(mb), name_(name) {

  std::vector<llvm::Type *> arg_types;
  for (auto &arg : args) {
    arg_types.push_back(arg.type);
  }

  auto *fn_type = llvm::FunctionType::get(ret_type, arg_types, false);
  function_ = llvm::Function::Create(
      fn_type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name,
      &mb_.module());

  auto arg_iter = args.begin();
  for (auto iter = function_->arg_begin(), end = function_->arg_end();
       iter != end; iter++, arg_iter++) {
    iter->setName(arg_iter->name);
  }

  function_->addFnAttr("target-cpu", llvm::sys::getHostCPUName());

  auto block = llvm::BasicBlock::Create(mb_.context(), "entry", function_);
  mb_.ir_builder().SetInsertPoint(block);

  mb_.debug_info().EnterFunction(function_);
}

llvm::Value *FunctionBuilder::GetArgumentByName(const std::string &name) {
  for (auto &arg : function_->args()) {
    if (arg.getName().equals(name)) {
      return &arg;
    }
  }
  return nullptr;
}

llvm::Value *FunctionBuilder::GetArgumentByPosition(uint32_t index) {
  uint32_t pos = 0;
  for (auto arg_iter = function_->arg_begin(), end = function_->arg_end();
       arg_iter != end; ++arg_iter, ++pos) {
    if (pos == index) {
      return &*arg_iter;
    }
  }
  return nullptr;
}

void FunctionBuilder::Finish(llvm::Value *ret_value) {
  if (ret_value != nullptr) {
    mb_.ir_builder().CreateRet(ret_value);
  } else {
    mb_.ir_builder().CreateRetVoid();
  }
  mb_.debug_info().ExitFunction(ret_value);
}

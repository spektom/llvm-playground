#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#include "jit.h"
#include "optimizer.h"

Jit::Jit(std::unique_ptr<llvm::TargetMachine> target_machine,
         const std::string &cache_dir)
    : exec_session_(std::make_unique<llvm::orc::ExecutionSession>()),
      target_machine_(std::move(target_machine)),
      obj_cache_(std::make_unique<ObjectCache>(cache_dir)),
      gdb_listener_(llvm::JITEventListener::createGDBRegistrationListener()),
      linking_layer_(*exec_session_, CreateMemoryManagerFtor(),
                     CreateNotifyLoadedFtor()),
      compile_layer_(
          *exec_session_, linking_layer_,
          llvm::orc::SimpleCompiler(*this->target_machine_, obj_cache_.get())),
      optimize_layer_(*exec_session_, compile_layer_) {
  if (auto resolver = CreateHostProcessResolver()) {
    exec_session_->getMainJITDylib().setGenerator(std::move(resolver));
  }
}

llvm::orc::JITDylib::GeneratorFunction Jit::CreateHostProcessResolver() {
  llvm::DataLayout data_layout = target_machine_->createDataLayout();
  llvm::Expected<llvm::orc::JITDylib::GeneratorFunction> resolver =
      llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
          data_layout);

  if (!resolver) {
    exec_session_->reportError(resolver.takeError());
    return nullptr;
  }

  if (!*resolver) {
    exec_session_->reportError(llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "Generator function for host process symbols must not be null"));
    return nullptr;
  }

  return *resolver;
}

using GetMemoryManagerFunction =
    llvm::orc::RTDyldObjectLinkingLayer::GetMemoryManagerFunction;

GetMemoryManagerFunction Jit::CreateMemoryManagerFtor() {
  return []() -> GetMemoryManagerFunction::result_type {
    return std::make_unique<llvm::SectionMemoryManager>();
  };
}

llvm::orc::RTDyldObjectLinkingLayer::NotifyLoadedFunction
Jit::CreateNotifyLoadedFtor() {
  return std::bind(&llvm::JITEventListener::notifyObjectLoaded, gdb_listener_,
                   std::placeholders::_1, std::placeholders::_2,
                   std::placeholders::_3);
}

std::string Jit::Mangle(llvm::StringRef unmangled_name) {
  std::string mangled_name;
  {
    llvm::DataLayout data_layout = target_machine_->createDataLayout();
    llvm::raw_string_ostream mangled_name_stream(mangled_name);
    llvm::Mangler::getNameWithPrefix(mangled_name_stream, unmangled_name,
                                     data_layout);
  }
  return mangled_name;
}

llvm::Error Jit::ApplyDataLayout(llvm::Module &module) {
  llvm::DataLayout data_layout = target_machine_->createDataLayout();
  if (module.getDataLayout().isDefault()) {
    module.setDataLayout(data_layout);
  }

  if (module.getDataLayout() != data_layout) {
    return llvm::make_error<llvm::StringError>(
        "Added modules have incompatible data layouts",
        llvm::inconvertibleErrorCode());
  }

  return llvm::Error::success();
}

llvm::Error Jit::SubmitModule(std::unique_ptr<llvm::Module> module,
                              std::unique_ptr<llvm::LLVMContext> context,
                              OptimizationLevel optimization_level,
                              bool add_to_cache) {
  if (add_to_cache) {
    obj_cache_->SetCacheModuleName(*module);
  }

  auto obj = obj_cache_->GetCachedObject(*module);
  if (!obj) {
    module.~unique_ptr();
    return obj.takeError();
  }

  if (obj->hasValue()) {
    module.~unique_ptr();
    return linking_layer_.add(exec_session_->getMainJITDylib(),
                              std::move(obj->getValue()));
  }

  LLVM_DEBUG(dbgs() << "Submit IR module:\n\n" << *M << "\n\n");

  if (auto err = ApplyDataLayout(*module)) {
    return err;
  }

  optimize_layer_.setTransform(Optimizer(optimization_level));

  return optimize_layer_.add(
      exec_session_->getMainJITDylib(),
      llvm::orc::ThreadSafeModule(std::move(module), std::move(context)),
      exec_session_->allocateVModule());
}

llvm::Expected<llvm::JITTargetAddress>
Jit::GetFunctionAddr(llvm::StringRef name) {
  llvm::orc::SymbolStringPtr name_ptr = exec_session_->intern(Mangle(name));
  llvm::orc::JITDylibSearchList JDs{{&exec_session_->getMainJITDylib(), true}};

  llvm::Expected<llvm::JITEvaluatedSymbol> symbol =
      exec_session_->lookup(JDs, name_ptr);
  if (!symbol) {
    return symbol.takeError();
  }

  llvm::JITTargetAddress addr = symbol->getAddress();
  if (!addr) {
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "'%s' evaluated to nullptr", name.data());
  }

  return addr;
}

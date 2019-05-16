#ifndef JIT_H_
#define JIT_H_

#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/JITEventListener.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/ObjectCache.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/Error.h>
#include <llvm/Target/TargetMachine.h>

#include <functional>
#include <memory>
#include <string>

#include "object_cache.h"
#include "optimization_level.h"

class Jit {
public:
  Jit(std::unique_ptr<llvm::TargetMachine> target_machine,
      const std::string &cache_dir = "");

  // Not a value type.
  Jit(const Jit &) = delete;
  Jit &operator=(const Jit &) = delete;
  Jit(Jit &&) = delete;
  Jit &operator=(Jit &&) = delete;

  llvm::Error SubmitModule(std::unique_ptr<llvm::Module> module,
                           std::unique_ptr<llvm::LLVMContext> context,
                           OptimizationLevel optimization_level,
                           bool add_to_cache);

  template <class Signature_t>
  llvm::Expected<std::function<Signature_t>> GetFunction(llvm::StringRef name) {
    if (auto addr = GetFunctionAddr(name)) {
      return std::function<Signature_t>(
          llvm::jitTargetAddressToPointer<Signature_t *>(*addr));
    } else {
      return addr.takeError();
    }
  }

private:
  std::unique_ptr<llvm::orc::ExecutionSession> exec_session_;
  std::unique_ptr<llvm::TargetMachine> target_machine_;

  std::unique_ptr<ObjectCache> obj_cache_;
  llvm::JITEventListener *gdb_listener_;

  llvm::orc::RTDyldObjectLinkingLayer linking_layer_;
  llvm::orc::IRCompileLayer compile_layer_;
  llvm::orc::IRTransformLayer optimize_layer_;

  llvm::orc::JITDylib::GeneratorFunction CreateHostProcessResolver();

  llvm::orc::RTDyldObjectLinkingLayer::GetMemoryManagerFunction
  CreateMemoryManagerFtor();

  llvm::orc::RTDyldObjectLinkingLayer::NotifyLoadedFunction
  CreateNotifyLoadedFtor();

  std::string Mangle(llvm::StringRef unmangled_name);
  llvm::Error ApplyDataLayout(llvm::Module &module);

  llvm::Expected<llvm::JITTargetAddress> GetFunctionAddr(llvm::StringRef name);
};

#endif /* JIT_H_ */

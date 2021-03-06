#ifndef JIT_COMPILER_H_
#define JIT_COMPILER_H_

#include <llvm/ExecutionEngine/JITEventListener.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>

#include <unordered_map>

#include "macros.h"

class JitCompiler {
public:
  JitCompiler();
  DISALLOW_COPY_AND_MOVE(JitCompiler);
  ~JitCompiler();

  void AddSymbol(const std::string &, void *);
  void AddModule(std::unique_ptr<llvm::Module>,
                 std::unique_ptr<llvm::LLVMContext>);
  void *GetFuncPtr(const std::string &);

  const llvm::DataLayout &data_layout() const { return data_layout_; }

private:
  llvm::Expected<llvm::orc::ThreadSafeModule>
  OptimizeModule(llvm::orc::ThreadSafeModule,
                 const llvm::orc::MaterializationResponsibility &);

private:
  explicit JitCompiler(llvm::orc::JITTargetMachineBuilder);

  llvm::orc::ExecutionSession session_;
  std::unique_ptr<llvm::TargetMachine> target_machine_;
  const llvm::DataLayout data_layout_;

  llvm::orc::RTDyldObjectLinkingLayer object_layer_;
  llvm::orc::IRCompileLayer compile_layer_;
  llvm::orc::IRTransformLayer optimize_layer_;

  std::vector<llvm::orc::VModuleKey> loaded_modules_;
  std::unordered_map<std::string, uintptr_t> external_symbols_;
  llvm::orc::DynamicLibrarySearchGenerator dynlib_generator_;
  llvm::orc::MangleAndInterner mangle_;

  llvm::JITEventListener *gdb_listener_;
};

#endif /* JIT_COMPILER_H_ */

#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#include "error.h"
#include "jit_compiler.h"
#include "util.h"

JitCompiler::JitCompiler(llvm::orc::JITTargetMachineBuilder tmb)
    : data_layout_(ThrowOnError(tmb.getDefaultDataLayoutForTarget())),
      target_machine_(ThrowOnError(tmb.createTargetMachine())),
      object_layer_(
          session_,
          [] { return std::make_unique<llvm::SectionMemoryManager>(); },
          [this](llvm::orc::VModuleKey vk, llvm::object::ObjectFile const &obj,
                 llvm::RuntimeDyld::LoadedObjectInfo const &info) {
            if (gdb_listener_) {
              gdb_listener_->notifyObjectLoaded(vk, obj, info);
            }
            loaded_modules_.emplace_back(vk);
          }),
      compile_layer_(session_, object_layer_,
                     llvm::orc::ConcurrentIRCompiler(std::move(tmb))),
      optimize_layer_(
          session_, compile_layer_,
          [this](llvm::orc::ThreadSafeModule tsm,
                 llvm::orc::MaterializationResponsibility const &mr) {
            return OptimizeModule(std::move(tsm), mr);
          }),
      gdb_listener_(llvm::JITEventListener::createGDBRegistrationListener()),
      dynlib_generator_(ThrowOnError(
          llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
              data_layout_))) {

  session_.getMainJITDylib().setGenerator(
      [this](llvm::orc::JITDylib &jd, llvm::orc::SymbolNameSet const &Names)
          -> llvm::orc::SymbolNameSet {
        auto added = llvm::orc::SymbolNameSet{};
        auto remaining = llvm::orc::SymbolNameSet{};
        auto new_symbols = llvm::orc::SymbolMap{};

        for (auto &name : Names) {
          auto it = external_symbols_.find(std::string(*name));
          if (it == external_symbols_.end()) {
            remaining.insert(name);
            continue;
          }
          added.insert(name);
          new_symbols[name] =
              llvm::JITEvaluatedSymbol(llvm::JITTargetAddress{it->second},
                                       llvm::JITSymbolFlags::Exported);
        }
        ThrowOnError(
            jd.define(llvm::orc::absoluteSymbols(std::move(new_symbols))));
        if (!remaining.empty()) {
          auto dynlib_added = dynlib_generator_(jd, remaining);
          added.insert(dynlib_added.begin(), dynlib_added.end());
        }
        return added;
      });
}

JitCompiler::JitCompiler()
    : JitCompiler([] {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();

        auto tmb =
            ThrowOnError(llvm::orc::JITTargetMachineBuilder::detectHost());
        tmb.setCodeGenOptLevel(llvm::CodeGenOpt::Aggressive);
        tmb.setCPU(llvm::sys::getHostCPUName());
        return tmb;
      }()) {}

JitCompiler::~JitCompiler() {
  for (auto vk : loaded_modules_) {
    gdb_listener_->notifyFreeingObject(vk);
  }
}

llvm::Expected<llvm::orc::ThreadSafeModule>
JitCompiler::OptimizeModule(llvm::orc::ThreadSafeModule tsm,
                            llvm::orc::MaterializationResponsibility const &) {

  auto module = tsm.getModule();
  auto target_triple = target_machine_->getTargetTriple();

  module->setDataLayout(data_layout_);
  module->setTargetTriple(target_triple.str());

  auto library_info =
      std::make_unique<llvm::TargetLibraryInfoImpl>(target_triple);
  auto function_passes = llvm::legacy::FunctionPassManager(module);
  auto module_passes = llvm::legacy::PassManager();

  auto builder = llvm::PassManagerBuilder{};
  builder.OptLevel = 3;
  builder.SizeLevel = 0;
  builder.Inliner = llvm::createFunctionInliningPass();
  builder.MergeFunctions = true;
  builder.LoopVectorize = true;
  builder.SLPVectorize = true;
  builder.DisableUnrollLoops = false;
  builder.RerollLoops = true;
  builder.LibraryInfo = new llvm::TargetLibraryInfoImpl(target_triple);

  function_passes.add(llvm::createTargetTransformInfoWrapperPass(
      target_machine_->getTargetIRAnalysis()));
  module_passes.add(llvm::createTargetTransformInfoWrapperPass(
      target_machine_->getTargetIRAnalysis()));

  target_machine_->adjustPassManager(builder);

  builder.populateFunctionPassManager(function_passes);
  builder.populateModulePassManager(module_passes);

  function_passes.doInitialization();
  for (auto &func : *module) {
    function_passes.run(func);
  }
  function_passes.doFinalization();

  module_passes.run(*module);
  return tsm;
}

void JitCompiler::AddSymbol(std::string const &name, void *address) {
  external_symbols_[name] = reinterpret_cast<uintptr_t>(address);
}

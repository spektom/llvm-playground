#include "Optimizer.h"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>

llvm::Expected<llvm::orc::ThreadSafeModule> Optimizer::
operator()(llvm::orc::ThreadSafeModule thread_safe_module,
           const llvm::orc::MaterializationResponsibility &) {
  llvm::Module &module = *thread_safe_module.getModule();

  llvm::legacy::FunctionPassManager func_pass_manager(&module);
  builder.populateFunctionPassManager(func_pass_manager);

  func_pass_manager.doInitialization();
  for (llvm::Function &function : module) {
    func_pass_manager.run(function);
  }
  func_pass_manager.doFinalization();

  llvm::legacy::PassManager module_pass_manager;
  builder.populateModulePassManager(module_pass_manager);
  module_pass_manager.run(module);

  LLVM_DEBUG(dbgs() << "Optimized IR module:\n\n" << module << "\n\n");

  return std::move(thread_safe_module);
}

#include "Optimizer.h"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>

llvm::Expected<llvm::orc::ThreadSafeModule> Optimizer::
operator()(llvm::orc::ThreadSafeModule TSM,
           const llvm::orc::MaterializationResponsibility &) {
  llvm::Module &M = *TSM.getModule();

  llvm::legacy::FunctionPassManager FPM(&M);
  B.populateFunctionPassManager(FPM);

  FPM.doInitialization();
  for (llvm::Function &F : M) {
    FPM.run(F);
  }
  FPM.doFinalization();

  llvm::legacy::PassManager MPM;
  B.populateModulePassManager(MPM);
  MPM.run(M);

  LLVM_DEBUG(dbgs() << "Optimized IR module:\n\n" << M << "\n\n");

  return std::move(TSM);
}

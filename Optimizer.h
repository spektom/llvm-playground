#ifndef OPTIMIZER_H_
#define OPTIMIZER_H_

#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/Error.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

class Optimizer {
public:
  Optimizer(unsigned optimization_level) {
    builder.OptLevel = optimization_level;
  }

  llvm::Expected<llvm::orc::ThreadSafeModule>
  operator()(llvm::orc::ThreadSafeModule module,
             const llvm::orc::MaterializationResponsibility &);

private:
  llvm::PassManagerBuilder builder;
};

#endif /* OPTIMIZER_H_ */

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include <llvm/Support/Debug.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>

#include "codegen.h"

Codegen::Codegen(int &argc, char **&argv) : init_(argc, argv) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  std::unique_ptr<llvm::TargetMachine> target_machine(
      llvm::EngineBuilder().selectTarget());

  LLVM_DEBUG(llvm::dbgs() << "Initialized Jit for host target: "
                          << target_machine->getTargetTriple().normalize()
                          << "\n\n");

  jit_ = std::make_unique<Jit>(std::move(target_machine), "/tmp/codegen");
}

std::unique_ptr<CodeContext>
Codegen::CreateContext(const std::string &module_name,
                       OptimizationLevel optimization_level) {
  return std::move(
      std::make_unique<CodeContext>(module_name, optimization_level));
}

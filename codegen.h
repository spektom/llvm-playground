#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <llvm/Support/InitLLVM.h>

#include <memory>
#include <string>

#include "code_context.h"
#include "jit.h"

class Codegen {
public:
  Codegen(int &argc, char **&argv);

  std::unique_ptr<CodeContext>
  CreateContext(const std::string &module_name,
                OptimizationLevel optimization_level = OptimizationLevel::O2);

private:
  llvm::InitLLVM init_;
  std::unique_ptr<Jit> jit_;
};

#endif /* CODEGEN_H_ */

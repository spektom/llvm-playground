#include <iostream>
#include <llvm/Support/InitLLVM.h>

#include "function.h"
#include "jit_compiler.h"
#include "module.h"

int main(int argc, char **argv) {
  llvm::InitLLVM init(argc, argv);
  JitCompiler jc;
  ModuleBuilder mb(jc, "MyModule");
  FunctionBuilder fb(mb, "foo", mb.codegen().Int8Type(), {});
  fb.Finish(mb.codegen().Const8(1));
  mb.Finish();

  std::cout << reinterpret_cast<uint64_t>(jc.FindFunction("foo")) << std::endl;
}

#include <iostream>
#include <llvm/Support/InitLLVM.h>

#include "function.h"
#include "jit_compiler.h"
#include "module.h"

int main(int argc, char **argv) {
  llvm::InitLLVM init(argc, argv);

  JitCompiler jc;
  ModuleBuilder mb(jc, "MyModule");

  auto *mul_func = +[](int32_t arg1) { return arg1 * arg1; };

  auto mul = mb.RegExtFunc("mul", mb.types().Int32(), {{mb.types().Int32()}},
                           reinterpret_cast<void *>(mul_func));

  auto foo =
      mb.CreateFunc("foo", mb.types().Int32(), {{"arg1", mb.types().Int32()}});
  auto ret_val = mb.statements().Call(mul, {foo.GetArgByName("arg1")});
  foo.Return(ret_val);
  mb.Build();

  auto foo_ptr = reinterpret_cast<int32_t (*)(int32_t)>(jc.GetFuncPtr("foo"));
  std::cout << std::to_string(foo_ptr(88)) << std::endl;
}

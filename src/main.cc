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

  auto mul = mb.RegisterExternalFunction(
      "mul", mb.types().Int32(), {{"arg1", mb.types().Int32()}}, reinterpret_cast<void*>(mul_func));

  mb.EnterFunction("foo", mb.types().Int8(), {});
  auto c = mb.constants().Get((int8_t)1);
  mb.ExitFunction(c);
  mb.Finish();

  auto foo = reinterpret_cast<int8_t (*)()>(jc.GetFunction("foo"));
  auto mul_ptr = reinterpret_cast<int32_t (*)(int32_t)>(jc.GetFunction("mul"));

  std::cout << std::to_string(foo()) << std::endl;
  std::cout << std::to_string(mul_ptr(88)) << std::endl;
}

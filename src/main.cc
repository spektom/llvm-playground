#include <iostream>
#include <llvm/Support/InitLLVM.h>

#include "function.h"
#include "jit_compiler.h"
#include "module.h"

int main(int argc, char **argv) {
  llvm::InitLLVM init(argc, argv);

  JitCompiler jc;
  ModuleBuilder mb(jc, "MyModule");

  auto *mul_func = +[](int32_t arg1, int32_t arg2) { return arg1 * arg2; };

  auto mul = mb.RegExtFunc("mul", mb.types().Int32(),
                           {{mb.types().Int32()}, {mb.types().Int32()}},
                           reinterpret_cast<void *>(mul_func));

  auto foo =
      mb.GetFunc("foo", mb.types().Int32(), {{"arg1", mb.types().Int32()}});

  auto strct =
      mb.GetStruct("Abc", {{"f1", mb.types().Array(mb.types().Int32(), 100)}});

  auto arr = mb.GetArray(strct.GetPtr("f1"));

  arr.Set(50, mb.constants().Get((uint32_t)5));
  arr.Set(51, mb.constants().Get((uint32_t)3));

  auto arr2 = mb.GetArray(mb.types().Int32(), 200);
  arr2.Set(100, foo.GetArgByName("arg1"));

  foo.Return(mb.statements().Call(mul, {arr.Get(50), arr.Get(51)}));
  std::cout << mb.GetIR() << std::endl;
  mb.Build();

  auto foo_ptr = reinterpret_cast<int32_t (*)(int32_t)>(jc.GetFuncPtr("foo"));
  std::cout << std::to_string(foo_ptr(10)) << std::endl;
}

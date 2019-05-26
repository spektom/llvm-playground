#ifndef MODULE_H_
#define MODULE_H_

#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/IR/DataLayout.h>

#include "macros.h"
#include "function.h"

class Module {
  friend class ModuleBuilder;

public:
  DISALLOW_COPY_AND_MOVE(Module);

  template <typename ReturnType, typename... Arguments>
  auto GetAddress(Function<ReturnType, Arguments...> const &fn) {
    return reinterpret_cast<ReturnType (*)(Arguments...)>(
        GetAddress(fn.name()));
  }

private:
  Module(llvm::orc::ExecutionSession &, llvm::DataLayout const &);
  void *GetAddress(std::string const &);

private:
  llvm::orc::ExecutionSession *session_;
  llvm::orc::MangleAndInterner mangle_;
};

#endif /* MODULE_H_ */

#ifndef FUNCTION_BUILDER_H_
#define FUNCTION_BUILDER_H_

#include <llvm/IR/Function.h>
#include <sstream>

#include "function.h"
#include "module_builder.h"

template <typename ReturnType, typename... Arguments>
class FunctionBuilder<ReturnType(Arguments...)> {
public:
  FunctionBuilder(ModuleBuilder &mb) : mb_(mb) {}
};

#endif /* FUNCTION_BUILDER_H_ */

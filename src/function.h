#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <llvm/IR/Function.h>

template <typename ReturnType, typename... Arguments> class Function {
public:
  Function(const std::string &name, llvm::Function *fn)
      : name_(name), function_(fn) {}

  operator llvm::Function *() const { return function_; }
  std::string const &name() const { return name_; }

private:
  std::string name_;
  llvm::Function *function_;
};

#endif /* FUNCTION_H_ */

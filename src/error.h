#ifndef ERROR_H_
#define ERROR_H_

#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>

class LLVMError : public std::runtime_error {
public:
  explicit LLVMError(llvm::Error err)
      : std::runtime_error([&] {
          auto str = std::string{"LLVM Error: "};
          auto os = llvm::raw_string_ostream(str);
          os << err;
          os.str();
          return str;
        }()) {}
};

inline void ThrowOnError(llvm::Error err) {
  if (err) {
    throw LLVMError(std::move(err));
  }
}

template <typename T> T ThrowOnError(llvm::Expected<T> value) {
  if (!value) {
    throw LLVMError(value.takeError());
  }
  return std::move(*value);
}

#endif /* ERROR_H_ */

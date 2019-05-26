#ifndef DEBUG_INFO_H_
#define DEBUG_INFO_H_

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Module.h>

#include <filesystem>
#include <sstream>

// Maintains generated C++ code along with LLVM debug info
// for a module to facilitate easier debugging
class DebugInfo {
private:
  unsigned INDENT_SIZE = 4;

public:
  DebugInfo(llvm::Module &);
  ~DebugInfo();

  void EnterFunction(llvm::Function &);
  void ExitFunction();
  void ExitModule();

private:
  void AddLine(const std::string &);
  void EnterScope() { indent_ += INDENT_SIZE; }
  void LeaveScope() { indent_ -= INDENT_SIZE; }
  llvm::DebugLoc GetDebugLocation();
  llvm::DIType *GetDIType(llvm::Type *type);

private:
  std::stringstream source_code_;
  unsigned line_no_;
  unsigned indent_;
  bool enabled_;

  std::filesystem::path source_file_;
  llvm::Module &module_;
  llvm::DIBuilder dbg_builder_;
  llvm::DIFile *dbg_file_;
  llvm::DIScope *dbg_scope_;
};

#endif /* DEBUG_INFO_H_ */

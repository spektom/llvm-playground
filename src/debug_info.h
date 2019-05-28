#ifndef DEBUG_INFO_H_
#define DEBUG_INFO_H_

#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/DebugLoc.h>

#include <filesystem>
#include <sstream>

#include "macros.h"

class ModuleBuilder;

// Maintains generated C++ code along with LLVM debug info
// for a module to facilitate easier debugging
class DebugInfo {
private:
  unsigned INDENT_SIZE = 4;

public:
  DebugInfo(ModuleBuilder &);
  DISALLOW_COPY_AND_MOVE(DebugInfo);
  ~DebugInfo();

  void EnterFunction(llvm::Function *);
  void ExitFunction(llvm::Value *);
  void ExitModule();

private:
  void AddLine(const std::string &);
  void EnterScope() { indent_ += INDENT_SIZE; }
  void LeaveScope() { indent_ -= INDENT_SIZE; }
  llvm::DIType *GetDIType(llvm::Type *type);
  llvm::DebugLoc GetDebugLocation();

private:
  std::stringstream source_code_;
  unsigned line_no_;
  unsigned indent_;
  bool enabled_;

  ModuleBuilder &mb_;
  std::filesystem::path source_file_;
  llvm::DIBuilder dbg_builder_;
  llvm::DIFile *dbg_file_;
  llvm::DIScope *dbg_scope_;
};

#endif /* DEBUG_INFO_H_ */

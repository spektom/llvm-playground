#include "module.h"
#include "error.h"

Module::Module(llvm::orc::ExecutionSession &session, llvm::DataLayout const &dl)
    : session_(&session), mangle_(session, dl) {}

void *Module::GetAddress(std::string const &name) {
  auto address = ThrowOnError(session_->lookup({&session_->getMainJITDylib()},
                                               mangle_(name)))
                     .getAddress();
  return reinterpret_cast<void *>(address);
}

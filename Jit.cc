#include "Jit.h"
#include "Optimizer.h"

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

Jit::Jit(std::unique_ptr<llvm::TargetMachine> TM, const std::string &CacheDir)
    : ES(std::make_unique<llvm::orc::ExecutionSession>()), TM(std::move(TM)),
      ObjCache(std::make_unique<ObjectCache>(CacheDir)),
      GDBListener(llvm::JITEventListener::createGDBRegistrationListener()),
      ObjLinkingLayer(*ES, createMemoryManagerFtor(), createNotifyLoadedFtor()),
      CompileLayer(*ES, ObjLinkingLayer,
                   llvm::orc::SimpleCompiler(*this->TM, ObjCache.get())),
      OptimizeLayer(*ES, CompileLayer) {
  if (auto R = createHostProcessResolver())
    ES->getMainJITDylib().setGenerator(std::move(R));
}

llvm::orc::JITDylib::GeneratorFunction Jit::createHostProcessResolver() {
  llvm::DataLayout DL = TM->createDataLayout();
  llvm::Expected<llvm::orc::JITDylib::GeneratorFunction> R =
      llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(DL);

  if (!R) {
    ES->reportError(R.takeError());
    return nullptr;
  }

  if (!*R) {
    ES->reportError(llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "Generator function for host process symbols must not be null"));
    return nullptr;
  }

  return *R;
}

using GetMemoryManagerFunction =
    llvm::orc::RTDyldObjectLinkingLayer::GetMemoryManagerFunction;

GetMemoryManagerFunction Jit::createMemoryManagerFtor() {
  return []() -> GetMemoryManagerFunction::result_type {
    return std::make_unique<llvm::SectionMemoryManager>();
  };
}

llvm::orc::RTDyldObjectLinkingLayer::NotifyLoadedFunction
Jit::createNotifyLoadedFtor() {
  using namespace std::placeholders;
  return std::bind(&llvm::JITEventListener::notifyObjectLoaded, GDBListener, _1,
                   _2, _3);
}

std::string Jit::mangle(llvm::StringRef UnmangledName) {
  std::string MangledName;
  {
    llvm::DataLayout DL = TM->createDataLayout();
    llvm::raw_string_ostream MangledNameStream(MangledName);
    llvm::Mangler::getNameWithPrefix(MangledNameStream, UnmangledName, DL);
  }
  return MangledName;
}

llvm::Error Jit::applyDataLayout(llvm::Module &M) {
  llvm::DataLayout DL = TM->createDataLayout();
  if (M.getDataLayout().isDefault()) {
    M.setDataLayout(DL);
  }

  if (M.getDataLayout() != DL) {
    return llvm::make_error<llvm::StringError>(
        "Added modules have incompatible data layouts",
        llvm::inconvertibleErrorCode());
  }

  return llvm::Error::success();
}

llvm::Error Jit::submitModule(std::unique_ptr<llvm::Module> M,
                              std::unique_ptr<llvm::LLVMContext> C,
                              unsigned OptLevel, bool AddToCache) {
  if (AddToCache)
    ObjCache->setCacheModuleName(*M);

  auto Obj = ObjCache->getCachedObject(*M);
  if (!Obj) {
    M.~unique_ptr();
    return Obj.takeError();
  }

  if (Obj->hasValue()) {
    M.~unique_ptr();
    return ObjLinkingLayer.add(ES->getMainJITDylib(),
                               std::move(Obj->getValue()));
  }

  LLVM_DEBUG(dbgs() << "Submit IR module:\n\n" << *M << "\n\n");

  if (auto Err = applyDataLayout(*M)) {
    return Err;
  }

  OptimizeLayer.setTransform(Optimizer(OptLevel));

  return OptimizeLayer.add(
      ES->getMainJITDylib(),
      llvm::orc::ThreadSafeModule(std::move(M), std::move(C)),
      ES->allocateVModule());
}

llvm::Expected<llvm::JITTargetAddress>
Jit::getFunctionAddr(llvm::StringRef Name) {
  llvm::orc::SymbolStringPtr NamePtr = ES->intern(mangle(Name));
  llvm::orc::JITDylibSearchList JDs{{&ES->getMainJITDylib(), true}};

  llvm::Expected<llvm::JITEvaluatedSymbol> S = ES->lookup(JDs, NamePtr);
  if (!S)
    return S.takeError();

  llvm::JITTargetAddress A = S->getAddress();
  if (!A)
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "'%s' evaluated to nullptr", Name.data());

  return A;
}

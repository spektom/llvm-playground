#include "ObjectCache.h"

#include <llvm/ADT/None.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/Twine.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

ObjectCache::ObjectCache(std::string Dir)
    : Enabled(!Dir.empty()), CacheDir(endWithSeparator(std::move(Dir))) {
  if (!llvm::sys::fs::exists(CacheDir)) {
    LLVM_DEBUG(llvm::dbgs() << format("Create new cache directory '%s'\n\n",
                                      CacheDir.c_str()));
    std::error_code EC = llvm::sys::fs::create_directories(CacheDir);
    if (EC) {
      LLVM_DEBUG(llvm::dbgs()
                 << format("Creating new cache directory '%s' failed with "
                           "error code %d; Caching disabled\n\n",
                           CacheDir.c_str(), EC.value()));
      Enabled = false;
    }
  }
}

std::string ObjectCache::endWithSeparator(std::string Path) {
  return Path.back() == '/' ? Path : Path + "/";
}

// Implements llvm::ObjectCache::notifyObjectCompiled, called from CompileLayer
void ObjectCache::notifyObjectCompiled(const llvm::Module *M,
                                       llvm::MemoryBufferRef Obj) {
  assert(M && "Caching requires module");

  auto R = getCacheFileName(M->getModuleIdentifier());
  if (!R.hasValue()) {
    return;
  }

  std::string F = std::move(R.getValue());
  if (auto EC =
          llvm::sys::fs::create_directories(llvm::sys::path::parent_path(F))) {
    LLVM_DEBUG(llvm::dbgs() << format(
                   "Writing cached object '%s' failed with error code %d\n\n",
                   F.c_str(), EC.value()));
    return;
  }

  std::error_code EC;
  llvm::raw_fd_ostream OS(F, EC, llvm::sys::fs::F_None);
  if (EC) {
    LLVM_DEBUG(llvm::dbgs() << format(
                   "Writing cached object '%s' failed with error code %d\n\n",
                   F.c_str(), EC.value()));
    return;
  }

  LLVM_DEBUG(llvm::dbgs() << format("Write cached object '%s'\n\n", F.c_str()));

  OS.write(Obj.getBufferStart(), Obj.getBufferSize());
  OS.close();
}

// Implements llvm::ObjectCache::getObject, called from CompileLayer
std::unique_ptr<llvm::MemoryBuffer>
ObjectCache::getObject(const llvm::Module *M) {
  assert(M && "Lookup requires module");

  auto R = getCachedObject(*M);
  if (!R) {
    logAllUnhandledErrors(R.takeError(), llvm::dbgs(), "ObjectCache: ");
    return nullptr; // Error
  }

  if (!R->hasValue()) {
    return nullptr; // No cache entry
  }

  return std::forward<std::unique_ptr<llvm::MemoryBuffer>>(R->getValue());
}

llvm::Expected<llvm::Optional<std::unique_ptr<llvm::MemoryBuffer>>>
ObjectCache::getCachedObject(const llvm::Module &M) const {
  auto R = getCacheFileName(M.getModuleIdentifier());
  if (!R.hasValue()) {
    return llvm::None;
  }

  std::string F = std::move(R.getValue());
  if (!llvm::sys::fs::exists(F)) {
    return llvm::None;
  }

  auto B = llvm::MemoryBuffer::getFile(F, -1, false);
  if (!B)
    return llvm::createStringError(
        B.getError(),
        "Reading cached object '%s' failed with error code %d\n\n", F.c_str(),
        B.getError().value());

  LLVM_DEBUG(llvm::dbgs() << format("Read cached object '%s'\n\n", F.c_str()));
  return std::forward<std::unique_ptr<llvm::MemoryBuffer>>(*B);
}

void ObjectCache::setCacheModuleName(llvm::Module &M) const {
  if (Enabled && !M.getName().startswith("file:"))
    M.setModuleIdentifier("file:" + M.getModuleIdentifier() + ".o");
}

llvm::Optional<std::string>
ObjectCache::getCacheFileName(llvm::StringRef ModID) const {
  if (!Enabled) {
    return llvm::None;
  }

  llvm::StringRef Prefix = "file:";
  if (!ModID.startswith(Prefix)) {
    return llvm::None;
  }

  std::string Name = llvm::Twine(CacheDir + ModID.substr(Prefix.size())).str();
  size_t DotPos = Name.rfind('.');
  if (DotPos != std::string::npos)
    Name.replace(DotPos, Name.size() - DotPos, ".o");

  return Name;
}

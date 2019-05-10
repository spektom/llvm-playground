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

ObjectCache::ObjectCache(std::string dir)
    : enabled(!dir.empty()), cache_dir(EndWithSeparator(std::move(dir))) {
  if (!llvm::sys::fs::exists(cache_dir)) {
    LLVM_DEBUG(llvm::dbgs() << format("Create new cache directory '%s'\n\n",
                                      cache_dir.c_str()));
    std::error_code error_code = llvm::sys::fs::create_directories(cache_dir);
    if (error_code) {
      LLVM_DEBUG(llvm::dbgs()
                 << format("Creating new cache directory '%s' failed with "
                           "error code %d; Caching disabled\n\n",
                           cache_dir.c_str(), error_code.value()));
      enabled = false;
    }
  }
}

std::string ObjectCache::EndWithSeparator(std::string path) {
  return path.back() == '/' ? path : path + "/";
}

// Implements llvm::ObjectCache::notifyObjectCompiled, called from CompileLayer
void ObjectCache::notifyObjectCompiled(const llvm::Module *module,
                                       llvm::MemoryBufferRef obj) {
  assert(module && "Caching requires module");

  auto file_name = GetCacheFileName(module->getModuleIdentifier());
  if (!file_name.hasValue()) {
    return;
  }

  std::string file = std::move(file_name.getValue());
  if (auto error_code = llvm::sys::fs::create_directories(
          llvm::sys::path::parent_path(file))) {
    LLVM_DEBUG(llvm::dbgs() << format(
                   "Writing cached object '%s' failed with error code %d\n\n",
                   file.c_str(), error_code.value()));
    return;
  }

  std::error_code error_code;
  llvm::raw_fd_ostream os(file, error_code, llvm::sys::fs::F_None);
  if (error_code) {
    LLVM_DEBUG(llvm::dbgs() << format(
                   "Writing cached object '%s' failed with error code %d\n\n",
                   file.c_str(), error_code.value()));
    return;
  }

  LLVM_DEBUG(
      llvm::dbgs() << format("Write cached object '%s'\n\n", file.c_str()));

  os.write(obj.getBufferStart(), obj.getBufferSize());
  os.close();
}

// Implements llvm::ObjectCache::getObject, called from CompileLayer
std::unique_ptr<llvm::MemoryBuffer>
ObjectCache::getObject(const llvm::Module *module) {
  assert(module && "Lookup requires module");

  auto obj = GetCachedObject(*module);
  if (!obj) {
    logAllUnhandledErrors(obj.takeError(), llvm::dbgs(), "ObjectCache: ");
    return nullptr; // Error
  }

  if (!obj->hasValue()) {
    return nullptr; // No cache entry
  }

  return std::forward<std::unique_ptr<llvm::MemoryBuffer>>(obj->getValue());
}

llvm::Expected<llvm::Optional<std::unique_ptr<llvm::MemoryBuffer>>>
ObjectCache::GetCachedObject(const llvm::Module &module) const {
  auto file_name = GetCacheFileName(module.getModuleIdentifier());
  if (!file_name.hasValue()) {
    return llvm::None;
  }

  std::string file = std::move(file_name.getValue());
  if (!llvm::sys::fs::exists(file)) {
    return llvm::None;
  }

  auto buf = llvm::MemoryBuffer::getFile(file, -1, false);
  if (!buf)
    return llvm::createStringError(
        buf.getError(),
        "Reading cached object '%s' failed with error code %d\n\n",
        file.c_str(), buf.getError().value());

  LLVM_DEBUG(
      llvm::dbgs() << format("Read cached object '%s'\n\n", file.c_str()));
  return std::forward<std::unique_ptr<llvm::MemoryBuffer>>(*buf);
}

void ObjectCache::SetCacheModuleName(llvm::Module &module) const {
  if (enabled && !module.getName().startswith("file:")) {
    module.setModuleIdentifier("file:" + module.getModuleIdentifier() + ".o");
  }
}

llvm::Optional<std::string>
ObjectCache::GetCacheFileName(llvm::StringRef module_id) const {
  if (!enabled) {
    return llvm::None;
  }

  llvm::StringRef prefix = "file:";
  if (!module_id.startswith(prefix)) {
    return llvm::None;
  }

  std::string name =
      llvm::Twine(cache_dir + module_id.substr(prefix.size())).str();
  size_t dot_pos = name.rfind('.');
  if (dot_pos != std::string::npos) {
    name.replace(dot_pos, name.size() - dot_pos, ".o");
  }
  return name;
}

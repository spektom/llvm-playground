#ifndef OBJECT_CACHE_H_
#define OBJECT_CACHE_H_

#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/ObjectCache.h>

#include <memory>
#include <string>

// Simple name-based lookup, based on lli's implementation
class ObjectCache : public llvm::ObjectCache {
public:
  ObjectCache(std::string Dir);
  ~ObjectCache() = default;

  void setCacheModuleName(llvm::Module &M) const;

  llvm::Expected<llvm::Optional<std::unique_ptr<llvm::MemoryBuffer>>>
  getCachedObject(const llvm::Module &M) const;

protected:
  std::unique_ptr<llvm::MemoryBuffer> getObject(const llvm::Module *M) override;
  void notifyObjectCompiled(const llvm::Module *M,
                            llvm::MemoryBufferRef Obj) override;

private:
  bool Enabled;
  std::string CacheDir;

  static std::string endWithSeparator(std::string Path);
  llvm::Optional<std::string> getCacheFileName(llvm::StringRef ModID) const;
};

#endif /* OBJECT_CACHE_H_ */

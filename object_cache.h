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
  ObjectCache(std::string dir);
  ~ObjectCache() = default;

  void SetCacheModuleName(llvm::Module &module) const;

  llvm::Expected<llvm::Optional<std::unique_ptr<llvm::MemoryBuffer>>>
  GetCachedObject(const llvm::Module &module) const;

protected:
  std::unique_ptr<llvm::MemoryBuffer> getObject(const llvm::Module *M) override;
  void notifyObjectCompiled(const llvm::Module *M,
                            llvm::MemoryBufferRef Obj) override;

private:
  bool enabled_;
  std::string cache_dir_;

  static std::string EndWithSeparator(std::string path);
  llvm::Optional<std::string> GetCacheFileName(llvm::StringRef module_id) const;
};

#endif /* OBJECT_CACHE_H_ */

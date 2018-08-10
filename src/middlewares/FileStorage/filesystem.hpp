#pragma once

#include <map>
#include <mutex>
#include "storage.hpp"
#include "storedfile.hpp"

namespace TinyCDN::Middleware::FileStorage {

class FilesystemStorage : public FileStorage {
private:
  std::ofstream META;
  static const fs::path linkDirName;
  //! Saves META properties
  void persist();

  mutable std::mutex mutex;

  std::atomic<fileId> storeUniqueId;
  //std::map<fileId, std::unique_ptr<std::mutex>> storeMutexes;
  std::map<fileId, std::shared_mutex> storeMutexes;
  //! Once the store reaches this amount of files, another folder will get created
  static const int storeFileThreshold;

  //std::map<fileId, std::unique_ptr<std::shared_mutex>> fileMutexes;
  std::map<fileId, std::shared_mutex> fileMutexes;

  fileId getUniqueStoreId();
  fileId getUniqueFileId();

public:

  //! Creates a directory for stored files
  void allocate();
  //! Deletes the storage directory
  void destroy();

  std::unique_ptr<StoredFile> lookup(fileId id);
  std::unique_ptr<StoredFile> add(std::unique_ptr<StoredFile> file);
  void remove(std::unique_ptr<StoredFile> file);

  FilesystemStorage(Size allocatedSize, fs::path location, bool preallocated);
  ~FilesystemStorage();
};

}

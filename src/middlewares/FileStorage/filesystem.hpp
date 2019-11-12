#pragma once

#include <map>
#include <mutex>
#include "storage.hpp"
#include "storedfile.hpp"

namespace TinyCDN::Middleware::FileStorage {

using StoreId = Id<32>;

//! Implementation of the FileStorage abstract class utilizing std::filesystem
class FilesystemStorage : public FileStorage {
private:
  StoreId currentStoreId;
  std::ofstream META;
  static const fs::path linkDirName;
  //! Saves META properties
  void persist();

  mutable std::mutex mutex;

  //std::map<fileId, std::unique_ptr<std::mutex>> storeMutexes;
  std::unordered_map<StoreId, std::shared_mutex, IdHasher> storeMutexes;
  //! Once the store reaches this amount of files, another folder will get created
  static const int storeFileThreshold;

  //std::map<fileId, std::unique_ptr<std::shared_mutex>> fileMutexes;
  std::unordered_map<FileId, std::shared_mutex, IdHasher> fileMutexes;

  StoreId getUniqueStoreId();
  FileId getUniqueFileId();

  IdFactory idFactory;

public:

  //! Creates a directory for stored files
  void allocate();
  //! Deletes the storage directory
  void destroy();

  std::unique_ptr<StoredFile> lookup(FileId id);
  std::unique_ptr<StoredFile> add(std::unique_ptr<StoredFile> file);
  void remove(std::unique_ptr<StoredFile> file);

  FilesystemStorage(Size allocatedSize, fs::path location, bool preallocated);
  ~FilesystemStorage();
};

}

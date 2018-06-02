#pragma once

#include "storage.hpp"
#include "storedfile.hpp"

namespace TinyCDN::Middleware::FileStorage {

class FilesystemStorage : public FileStorage {
private:
  std::ofstream META;
  static const fs::path linkDirName;

protected:
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

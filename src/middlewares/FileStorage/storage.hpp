#pragma once

#include <atomic>
#include <iostream>
#include <fstream>
#include <tuple>
#include "../../utility.hpp"
#include "storedfile.hpp"
#include "../exceptions.hpp"

namespace TinyCDN::Middleware::FileStorage {
using TinyCDN::Utility::Size;

namespace File = TinyCDN::Middleware::File;

//! A common interface between Volume instances and an underlying storage backend.
class FileStorage {
protected:
  //! How much has been allocated already
  std::unique_ptr<Size> allocatedSize;

public:
  //! The maximum allowed size
  Size size;
  fs::path location;
  const bool preallocated = false;

  inline Size getAllocatedSize() {
    return Size{*allocatedSize.get()};
  }

  //! Persistence of the fileUniqueId is implementation-specific
  virtual FileId getUniqueFileId() = 0;

  //! Storage backends override this method to allocate storage on the disk
  //! in whatever data format the storage backend uses.
  virtual void allocate() = 0;
  //! Completely removes this storage and all of its contained files
  virtual void destroy() = 0;

  //! Finds a StoredFile by FileId
  virtual std::unique_ptr<StoredFile> lookup(FileId id) = 0;
  //! Adds a StoredFile into storage
  virtual std::unique_ptr<StoredFile> add(std::unique_ptr<StoredFile> file) = 0;
  //! Removes a StoredFile from storage
  virtual void remove(std::unique_ptr<StoredFile> file) = 0;

  FileStorage(Size size, fs::path location, bool preallocated);
  FileStorage(const FileStorage&) = delete;
  virtual ~FileStorage() = 0;
};





}

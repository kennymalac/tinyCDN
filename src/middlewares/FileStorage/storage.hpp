#pragma once

#include <atomic>
#include <iostream>
#include <fstream>
#include <tuple>
#include "../../utility.hpp"
#include "storedfile.hpp"
#include "../exceptions.hpp"

namespace TinyCDN::Middleware::FileStorage {

namespace File = TinyCDN::Middleware::File;

using fileId = uint_fast32_t;
// Storage backends should implement this abstract class
class FileStorage {
protected:
  std::atomic<fileId> fileUniqueId;
  std::unique_ptr<Size> allocatedSize;
  //! Persistence of the fileUniqueId is implementation-specific
  virtual fileId getUniqueFileId() = 0;

public:
  // static size
  Size size;
  fs::path location;
  const bool preallocated = false;

  //! Storage backends override this method to allocate storage on the disk
  //! in whatever data format the storage backend uses.
  virtual void allocate() = 0;
  //! Completely removes this storage and all of its contained files
  virtual void destroy() = 0;

  virtual std::unique_ptr<StoredFile> lookup(fileId id) = 0;
  virtual std::unique_ptr<StoredFile> add(std::unique_ptr<StoredFile> file) = 0;
  virtual void remove(std::unique_ptr<StoredFile> file) = 0;

  //! Helper method for deducing a file size
  auto inline assignStoredFileSize(StoredFile newContentFile) {
    std::ifstream f;
    f.open(newContentFile.location, std::ios_base::binary | std::ios_base::in);
    if (!f.good() || f.eof() || !f.is_open()) {
      throw File::FileStorageException(0, "Content file cannot be opened", std::make_optional<StoredFile>(newContentFile));
    }
    f.seekg(0, std::ios_base::beg);
    std::ifstream::pos_type begin_pos = f.tellg();
    f.seekg(0, std::ios_base::end);
    return Size{static_cast<uintmax_t>(f.tellg() - begin_pos)};
  }

  virtual std::unique_ptr<StoredFile> createStoredFile(fs::path tmpfile, Size fileSize, bool temporary);

  FileStorage(Size size, fs::path location, bool preallocated);
  virtual ~FileStorage() = 0;
};





}

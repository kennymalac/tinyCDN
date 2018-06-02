#include "storage.hpp"
#include "storedfile.hpp"


namespace TinyCDN::Middleware::FileStorage {

FileStorage::FileStorage(Size allocatedSize, fs::path location, bool preallocated)
  : allocatedSize(allocatedSize), location(location), preallocated(preallocated) {
}

FileStorage::~FileStorage() {};

std::unique_ptr<StoredFile> FileStorage::createStoredFile(fs::path tmpFile, Size fileSize, bool temporary) {
   // tmpfile is the uploaded file stored in /tmp/
   auto temporaryFile = std::make_unique<StoredFile>(fileSize, tmpFile, temporary);
   return temporaryFile;
}
}

#include "storage.hpp"
#include "storedfile.hpp"


namespace TinyCDN::Middleware::FileStorage {

FileStorage::FileStorage(Size allocatedSize, fs::path location, bool preallocated)
  : allocatedSize(allocatedSize), location(location), preallocated(preallocated) {
}

std::unique_ptr<StoredFile> FileStorage::createStoredFile(fs::path tmpfile, Size fileSize, bool temporary) {
   // tmpfile is the uploaded file stored in /tmp/
   auto temporaryFile = std::make_unique<StoredFile>(fileSize, location, temporary);
   return temporaryFile;
}
}

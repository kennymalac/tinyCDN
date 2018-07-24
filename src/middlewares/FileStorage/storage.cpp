#include "storage.hpp"
#include "storedfile.hpp"


namespace TinyCDN::Middleware::FileStorage {

FileStorage::FileStorage(Size size, fs::path location, bool preallocated)
  : allocatedSize(std::make_unique<Size>(0)), size(size), location(location), preallocated(preallocated) {
}

FileStorage::~FileStorage() {};
}
